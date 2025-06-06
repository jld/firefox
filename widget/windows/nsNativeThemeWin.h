/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsNativeThemeWin_h
#define nsNativeThemeWin_h

#include <windows.h>

#include "mozilla/Maybe.h"
#include "mozilla/TimeStamp.h"
#include "Theme.h"
#include "nsLookAndFeel.h"
#include "nsUXThemeConstants.h"

namespace mozilla::widget {

class nsNativeThemeWin final : public Theme {
 protected:
  virtual ~nsNativeThemeWin() = default;

 public:
  // Whether we draw a non-native widget.
  //
  // We always draw scrollbars as non-native so that all of Firefox has
  // consistent scrollbar styles both in chrome and content (plus, the
  // non-native scrollbars support scrollbar-width, auto-darkening...).
  //
  // We draw other widgets as non-native when their color-scheme is dark.  In
  // that case (`BecauseColorMismatch`) we don't call into the non-native theme
  // for sizing information (GetWidgetPadding/Border and GetMinimumWidgetSize),
  // to avoid subtle sizing changes. The non-native theme can basically draw at
  // any size, so we prefer to have consistent sizing information.
  enum class NonNative { No, Always, BecauseColorMismatch };
  static bool IsWidgetAlwaysNonNative(nsIFrame*, StyleAppearance);
  NonNative IsWidgetNonNative(nsIFrame*, StyleAppearance);

  // The nsITheme interface.
  NS_IMETHOD DrawWidgetBackground(gfxContext* aContext, nsIFrame* aFrame,
                                  StyleAppearance aAppearance,
                                  const nsRect& aRect, const nsRect& aDirtyRect,
                                  DrawOverflow) override;

  bool CreateWebRenderCommandsForWidget(wr::DisplayListBuilder&,
                                        wr::IpcResourceUpdateQueue&,
                                        const layers::StackingContextHelper&,
                                        layers::RenderRootStateManager*,
                                        nsIFrame*, StyleAppearance,
                                        const nsRect&) override;

  [[nodiscard]] LayoutDeviceIntMargin GetWidgetBorder(
      nsDeviceContext* aContext, nsIFrame* aFrame,
      StyleAppearance aAppearance) override;

  bool GetWidgetPadding(nsDeviceContext* aContext, nsIFrame* aFrame,
                        StyleAppearance aAppearance,
                        LayoutDeviceIntMargin* aResult) override;

  virtual bool GetWidgetOverflow(nsDeviceContext* aContext, nsIFrame* aFrame,
                                 StyleAppearance aAppearance,
                                 nsRect* aOverflowRect) override;

  LayoutDeviceIntSize GetMinimumWidgetSize(
      nsPresContext* aPresContext, nsIFrame* aFrame,
      StyleAppearance aAppearance) override;

  virtual Transparency GetWidgetTransparency(
      nsIFrame* aFrame, StyleAppearance aAppearance) override;

  bool WidgetAttributeChangeRequiresRepaint(StyleAppearance aAppearance,
                                            nsAtom* aAttribute) override;

  NS_IMETHOD ThemeChanged() override;

  bool ThemeSupportsWidget(nsPresContext* aPresContext, nsIFrame* aFrame,
                           StyleAppearance aAppearance) override;

  bool ThemeDrawsFocusForWidget(nsIFrame*, StyleAppearance) override;

  bool ThemeWantsButtonInnerFocusRing() override { return true; }

  nsNativeThemeWin();

 protected:
  Maybe<UXThemeClass> GetThemeClass(StyleAppearance aAppearance);
  HANDLE GetTheme(StyleAppearance aAppearance);
  nsresult GetThemePartAndState(nsIFrame* aFrame, StyleAppearance aAppearance,
                                int32_t& aPart, int32_t& aState);
  nsresult ClassicGetThemePartAndState(nsIFrame* aFrame,
                                       StyleAppearance aAppearance,
                                       int32_t& aPart, int32_t& aState,
                                       bool& aFocused);
  nsresult ClassicDrawWidgetBackground(gfxContext* aContext, nsIFrame* aFrame,
                                       StyleAppearance aAppearance,
                                       const nsRect& aRect,
                                       const nsRect& aClipRect);
  [[nodiscard]] LayoutDeviceIntMargin ClassicGetWidgetBorder(
      nsDeviceContext* aContext, nsIFrame* aFrame, StyleAppearance aAppearance);
  bool ClassicGetWidgetPadding(nsDeviceContext* aContext, nsIFrame* aFrame,
                               StyleAppearance aAppearance,
                               LayoutDeviceIntMargin* aResult);
  LayoutDeviceIntSize ClassicGetMinimumWidgetSize(nsIFrame* aFrame,
                                                  StyleAppearance aAppearance);
  bool ClassicThemeSupportsWidget(nsIFrame* aFrame,
                                  StyleAppearance aAppearance);
  void DrawCheckedRect(HDC hdc, const RECT& rc, int32_t fore, int32_t back,
                       HBRUSH defaultBack);
  uint32_t GetWidgetNativeDrawingFlags(StyleAppearance aAppearance);
  int32_t StandardGetState(nsIFrame* aFrame, StyleAppearance aAppearance,
                           bool wantFocused);
  bool IsMenuActive(nsIFrame* aFrame, StyleAppearance aAppearance);
  RECT CalculateProgressOverlayRect(nsIFrame* aFrame, RECT* aWidgetRect,
                                    bool aIsVertical, bool aIsIndeterminate,
                                    bool aIsClassic);
  void DrawThemedProgressMeter(nsIFrame* aFrame, StyleAppearance aAppearance,
                               HANDLE aTheme, HDC aHdc, int aPart, int aState,
                               RECT* aWidgetRect, RECT* aClipRect);

  [[nodiscard]] LayoutDeviceIntMargin GetCachedWidgetBorder(
      HANDLE aTheme, UXThemeClass aThemeClass, StyleAppearance aAppearance,
      int32_t aPart, int32_t aState);

  nsresult GetCachedMinimumWidgetSize(nsIFrame* aFrame, HANDLE aTheme,
                                      UXThemeClass aThemeClass,
                                      StyleAppearance aAppearance,
                                      int32_t aPart, int32_t aState,
                                      int32_t aSizeReq,
                                      LayoutDeviceIntSize* aResult);

  SIZE GetCachedGutterSize(HANDLE theme);

 private:
  TimeStamp mProgressDeterminateTimeStamp;
  TimeStamp mProgressIndeterminateTimeStamp;

  // UXThemeClass::NumClasses * THEME_PART_DISTINCT_VALUE_COUNT is about 800 at
  // the time of writing this, and nsIntMargin is 16 bytes wide, which makes
  // this cache (1/8 + 16) * 800 bytes, or about ~12KB. We could probably
  // reduce this cache to 3KB by caching on the aAppearance value instead,
  // but there would be some uncacheable values, since we derive some theme
  // parts from other arguments.
  uint8_t mBorderCacheValid[(size_t(UXThemeClass::NumClasses) *
                                 THEME_PART_DISTINCT_VALUE_COUNT +
                             7) /
                            8];
  LayoutDeviceIntMargin mBorderCache[size_t(UXThemeClass::NumClasses) *
                                     THEME_PART_DISTINCT_VALUE_COUNT];

  // See the above not for mBorderCache and friends. However
  // LayoutDeviceIntSize is half the size of nsIntMargin, making the
  // cache roughly half as large. In total the caches should come to about 18KB.
  uint8_t mMinimumWidgetSizeCacheValid[(size_t(UXThemeClass::NumClasses) *
                                            THEME_PART_DISTINCT_VALUE_COUNT +
                                        7) /
                                       8];
  LayoutDeviceIntSize mMinimumWidgetSizeCache[size_t(UXThemeClass::NumClasses) *
                                              THEME_PART_DISTINCT_VALUE_COUNT];

  bool mGutterSizeCacheValid;
  SIZE mGutterSizeCache;
};

}  // namespace mozilla::widget

#endif
