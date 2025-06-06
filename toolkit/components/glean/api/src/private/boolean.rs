// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use inherent::inherent;
use std::sync::Arc;

use glean::{traits::Boolean, MetricIdentifier};

use super::{BaseMetricId, ChildMetricMeta, CommonMetricData, MetricId, MetricNamer};

use crate::ipc::{need_ipc, with_ipc_payload};

/// A boolean metric.
///
/// Records a simple true or false value.
#[derive(Clone)]
pub enum BooleanMetric {
    Parent {
        /// The metric's ID. Used for testing and profiler markers. Boolean
        /// metrics can be labeled, so we may have either a metric ID or
        /// sub-metric ID.
        id: MetricId,
        inner: Arc<glean::private::BooleanMetric>,
    },
    Child(ChildMetricMeta),
    UnorderedChild(ChildMetricMeta),
}

impl BooleanMetric {
    /// Create a new boolean metric.
    pub fn new(id: BaseMetricId, meta: CommonMetricData) -> Self {
        if need_ipc() {
            BooleanMetric::Child(ChildMetricMeta::from_common_metric_data(id, meta))
        } else {
            BooleanMetric::Parent {
                id: id.into(),
                inner: Arc::new(glean::private::BooleanMetric::new(meta)),
            }
        }
    }

    pub fn with_unordered_ipc(id: BaseMetricId, meta: CommonMetricData) -> Self {
        if need_ipc() {
            BooleanMetric::UnorderedChild(ChildMetricMeta::from_common_metric_data(id, meta))
        } else {
            Self::new(id, meta)
        }
    }

    #[cfg(test)]
    pub(crate) fn metric_id(&self) -> MetricId {
        match self {
            BooleanMetric::Parent { id, .. } => *id,
            BooleanMetric::UnorderedChild(meta) => (meta.id).into(),
            _ => panic!("Can't get a metric_id from a non-ipc-supporting child boolean metric."),
        }
    }

    #[cfg(test)]
    pub(crate) fn child_metric(&self) -> Self {
        match self {
            BooleanMetric::Parent { id, inner } => {
                // SAFETY: We can unwrap here, as this code is only run in the
                // context of a test. If this code is used elsewhere, the
                // `unwrap` should be replaced with proper error handling of
                // the `None` case.
                BooleanMetric::Child(ChildMetricMeta::from_metric_identifier(
                    id.base_metric_id().unwrap(),
                    inner.as_ref(),
                ))
            }
            _ => panic!("Can't get a child metric from a child metric"),
        }
    }
}

#[inherent]
impl Boolean for BooleanMetric {
    /// Set to the specified boolean value.
    ///
    /// ## Arguments
    ///
    /// * `value` - the value to set.
    pub fn set(&self, value: bool) {
        match self {
            #[allow(unused)]
            BooleanMetric::Parent { id, inner } => {
                #[cfg(feature = "with_gecko")]
                gecko_profiler::add_marker(
                    "Boolean::set",
                    super::profiler_utils::TelemetryProfilerCategory,
                    Default::default(),
                    super::profiler_utils::BooleanMetricMarker::<BooleanMetric>::new(
                        *id, None, value,
                    ),
                );
                inner.set(value);
            }
            BooleanMetric::Child(_) => {
                log::error!("Unable to set boolean metric in non-main process. This operation will be ignored.");
                // If we're in automation we can panic so the instrumentor knows they've gone wrong.
                // This is a deliberate violation of Glean's "metric APIs must not throw" design.
                assert!(!crate::ipc::is_in_automation(), "Attempted to set boolean metric in non-main process, which is forbidden. This panics in automation.");
                // TODO: Record an error.
            }
            BooleanMetric::UnorderedChild(meta) => {
                #[cfg(feature = "with_gecko")]
                gecko_profiler::add_marker(
                    "Boolean::set",
                    super::profiler_utils::TelemetryProfilerCategory,
                    Default::default(),
                    super::profiler_utils::BooleanMetricMarker::<BooleanMetric>::new(
                        meta.id.into(),
                        None,
                        value,
                    ),
                );
                with_ipc_payload(move |payload| {
                    if let Some(v) = payload.booleans.get_mut(&meta.id) {
                        *v = value;
                    } else {
                        payload.booleans.insert(meta.id, value);
                    }
                });
            }
        }
    }

    /// **Test-only API.**
    ///
    /// Get the currently stored value as a boolean.
    /// This doesn't clear the stored value.
    ///
    /// ## Arguments
    ///
    /// * `ping_name` - the storage name to look into.
    ///
    /// ## Return value
    ///
    /// Returns the stored value or `None` if nothing stored.
    pub fn test_get_value<'a, S: Into<Option<&'a str>>>(&self, ping_name: S) -> Option<bool> {
        let ping_name = ping_name.into().map(|s| s.to_string());
        match self {
            BooleanMetric::Parent { id: _, inner } => inner.test_get_value(ping_name),
            _ => {
                panic!("Cannot get test value for boolean metric in non-main process!",)
            }
        }
    }

    /// **Exported for test purposes.**
    ///
    /// Gets the number of recorded errors for the given metric and error type.
    ///
    /// # Arguments
    ///
    /// * `error` - The type of error
    /// * `ping_name` - represents the optional name of the ping to retrieve the
    ///   metric for. Defaults to the first value in `send_in_pings`.
    ///
    /// # Returns
    ///
    /// The number of errors reported.
    pub fn test_get_num_recorded_errors(&self, error: glean::ErrorType) -> i32 {
        match self {
            BooleanMetric::Parent { id: _, inner } => inner.test_get_num_recorded_errors(error),
            _ => panic!(
                "Cannot get the number of recorded errors for boolean metric in non-main process!"
            ),
        }
    }
}

impl MetricNamer for BooleanMetric {
    fn get_metadata(&self) -> crate::private::MetricMetadata {
        crate::private::MetricMetadata::from_triple(match self {
            BooleanMetric::Parent { inner, .. } => inner.get_identifiers(),
            BooleanMetric::Child(meta) => meta.get_identifiers(),
            BooleanMetric::UnorderedChild(meta) => meta.get_identifiers(),
        })
    }
}

#[cfg(test)]
mod test {
    use crate::{common_test::*, ipc, metrics};

    #[test]
    fn sets_boolean_value() {
        let _lock = lock_test();

        let metric = &metrics::test_only_ipc::a_bool;
        metric.set(true);

        assert!(metric.test_get_value("test-ping").unwrap());
    }

    #[test]
    fn boolean_no_ipc() {
        // BooleanMetric doesn't support IPC.
        let _lock = lock_test();

        let parent_metric = &metrics::test_only_ipc::a_bool;

        parent_metric.set(false);

        {
            let child_metric = parent_metric.child_metric();

            // scope for need_ipc RAII
            let _raii = ipc::test_set_need_ipc(true);

            // Instrumentation calls do not panic.
            child_metric.set(true);

            // (They also shouldn't do anything,
            // but that's not something we can inspect in this test)
        }

        assert!(ipc::replay_from_buf(&ipc::take_buf().unwrap()).is_ok());

        assert!(
            false == parent_metric.test_get_value("test-ping").unwrap(),
            "Boolean metrics should only work in the parent process"
        );
    }

    #[test]
    fn boolean_unordered_ipc() {
        // BooleanMetric::UnorderedChild _does_ support IPC.
        let _lock = lock_test();

        let parent_metric = &metrics::test_only_ipc::an_unordered_bool;

        parent_metric.set(false);

        {
            let child_metric = parent_metric.child_metric();

            // scope for need_ipc RAII
            let _raii = ipc::test_set_need_ipc(true);

            child_metric.set(true);
        }

        assert!(ipc::replay_from_buf(&ipc::take_buf().unwrap()).is_ok());

        assert!(
            !parent_metric.test_get_value("test-ping").unwrap(),
            "Boolean metrics can unsafely work in child processes"
        );
    }
}
