# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

## Strings for the (add-ons) data consent feature.
## TODO: Bug 1956463 - expose these strings to localizers.
##
##
## Strings for data collection permissions in the permission prompt.

webext-perms-description-data-none = The developer says this extension doesn’t require data collection.

# Variables:
#    $permissions (String): a list of data collection permissions formatted with `Intl.ListFormat` using the "narrow" style.
webext-perms-description-data-some = The developer says this extension collects: { $permissions }

# Variables:
#    $permissions (String): a list of data collection permissions formatted with `Intl.ListFormat` using the "narrow" style.
webext-perms-description-data-some-update = The developer says the extension will collect: { $permissions }

# Variables:
#    $permissions (String): a list of data collection permissions formatted with `Intl.ListFormat` using the "narrow" style.
webext-perms-description-data-some-optional = The developer says the extension wants to collect: { $permissions }

# Variables:
#   $extension (String): replaced with the localized name of the extension.
webext-perms-update-text-with-data-collection = { $extension } requires new settings to update

webext-perms-update-listIntro-with-data-collection = Cancel to keep your current version and settings, or update to get the new version and approve the changes.

# Variables:
#   $extension (String): replaced with the localized name of the extension.
webext-perms-optional-text-with-data-collection = { $extension } requests additional settings

# Variables:
#   $extension (String): replaced with the localized name of the extension.
webext-perms-optional-text-with-data-collection-only = { $extension } requests additional data collection

## Short form to be used in lists or in a string (`webext-perms-description-data-some`)
## that formats some of these permissions below using `Intl.ListFormat`.
##
## This is used when the permissions are required.

webext-perms-description-data-short-authenticationInfo = authentication information
webext-perms-description-data-short-bookmarksInfo = bookmarks
webext-perms-description-data-short-browsingActivity = browsing activity
webext-perms-description-data-short-financialAndPaymentInfo = financial and payment information
webext-perms-description-data-short-healthInfo = health information
webext-perms-description-data-short-locationInfo = location
webext-perms-description-data-short-personalCommunications = personal communications
webext-perms-description-data-short-personallyIdentifyingInfo = personally identifying information
webext-perms-description-data-short-searchTerms = search terms
webext-perms-description-data-short-technicalAndInteraction = technical and interaction data
webext-perms-description-data-short-websiteActivity = website activity
webext-perms-description-data-short-websiteContent = website content

## Long form to be used in `about:addons` when these permissions are optional.

webext-perms-description-data-long-authenticationInfo = Share authentication information with extension developer
webext-perms-description-data-long-bookmarksInfo = Share bookmarks information with extension developer
webext-perms-description-data-long-browsingActivity = Share browsing activity with extension developer
webext-perms-description-data-long-financialAndPaymentInfo = Share financial and payment information with extension developer
webext-perms-description-data-long-healthInfo = Share health information with extension developer
webext-perms-description-data-long-locationInfo = Share location information with extension developer
webext-perms-description-data-long-personalCommunications = Share personal communications with extension developer
webext-perms-description-data-long-personallyIdentifyingInfo = Share personally identifying information with extension developer
webext-perms-description-data-long-searchTerms = Share search terms with extension developer
webext-perms-description-data-long-technicalAndInteraction = Share technical and interaction data with extension developer
webext-perms-description-data-long-websiteActivity = Share website activity with extension developer
webext-perms-description-data-long-websiteContent = Share website content with extension developer

## Headings for the Permissions tab in `about:addons`

addon-permissions-required-data-collection = Required data collection:
addon-permissions-optional-data-collection = Optional data collection:

# Name of the Permissions tab in `about:addons` when the data collection feature is enabled.
permissions-data-addon-button = Permissions and data

# This string is similar to `webext-perms-description-data-long-technicalAndInteraction`
# but it is used in the install prompt, and it needs an access key.
popup-notification-addon-technicalAndInteraction-checkbox =
    .label = Share technical and interaction data with extension developer
    .accesskey = S

# This string is used in the confirmation popup displayed after an extension has been installed.
appmenu-addon-post-install-message-with-data-collection = Update permissions and data preferences any time in the <a data-l10n-name="settings-link">extension settings</a>.

## Headers used in the webextension permissions dialog, inside the content.

webext-perms-header-data-collection-perms = Required data collection:
webext-perms-header-data-collection-is-none = Data collection:

# This is a header used in the add-ons "update" prompt, shown when the new
# version requires new data collection permissions.
webext-perms-header-update-data-collection-perms = New required data collection:

# This is a header used in the add-ons "optional" prompt, shown when the
# extension requests new data collection permissions programatically.
webext-perms-header-optional-data-collection-perms = New data collection:
