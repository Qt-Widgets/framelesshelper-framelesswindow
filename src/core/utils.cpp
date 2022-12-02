/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "utils.h"
#ifdef Q_OS_WINDOWS
#  include "winverhelper_p.h"
#endif // Q_OS_WINDOWS
#include <QtGui/qwindow.h>
#include <QtGui/qscreen.h>
#include <QtGui/qguiapplication.h>
#ifndef FRAMELESSHELPER_CORE_NO_PRIVATE
#  include <QtGui/private/qhighdpiscaling_p.h>
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
#  include <QtGui/qstylehints.h>
#elif ((QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
#  include <QtGui/qpa/qplatformtheme.h>
#  include <QtGui/private/qguiapplication_p.h>
#endif // (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcUtilsCommon, "wangwenx190.framelesshelper.core.utils.common")

#ifdef FRAMELESSHELPER_CORE_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcUtilsCommon)
#  define DEBUG qCDebug(lcUtilsCommon)
#  define WARNING qCWarning(lcUtilsCommon)
#  define CRITICAL qCCritical(lcUtilsCommon)
#endif

using namespace Global;

#ifndef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
struct FONT_ICON
{
    quint32 segoe = 0;
    quint32 micon = 0;
};

static const QHash<int, FONT_ICON> g_fontIconsTable = {
    {static_cast<int>(SystemButtonType::Unknown), {0x0000, 0x0000}},
    {static_cast<int>(SystemButtonType::WindowIcon), {0xE756, 0xEB06}},
    {static_cast<int>(SystemButtonType::Help), {0xE897, 0xEC04}},
    {static_cast<int>(SystemButtonType::Minimize), {0xE921, 0xEAE0}},
    {static_cast<int>(SystemButtonType::Maximize), {0xE922, 0xEADE}},
    {static_cast<int>(SystemButtonType::Restore), {0xE923, 0xEAE2}},
    {static_cast<int>(SystemButtonType::Close), {0xE8BB, 0xEADA}}
};
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE

Qt::CursorShape Utils::calculateCursorShape(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return Qt::ArrowCursor;
#else
    Q_ASSERT(window);
    if (!window) {
        return Qt::ArrowCursor;
    }
    if (window->visibility() != QWindow::Windowed) {
        return Qt::ArrowCursor;
    }
    const int x = pos.x();
    const int y = pos.y();
    const int w = window->width();
    const int h = window->height();
    if (((x < kDefaultResizeBorderThickness) && (y < kDefaultResizeBorderThickness))
        || ((x >= (w - kDefaultResizeBorderThickness)) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeFDiagCursor;
    }
    if (((x >= (w - kDefaultResizeBorderThickness)) && (y < kDefaultResizeBorderThickness))
        || ((x < kDefaultResizeBorderThickness) && (y >= (h - kDefaultResizeBorderThickness)))) {
        return Qt::SizeBDiagCursor;
    }
    if ((x < kDefaultResizeBorderThickness) || (x >= (w - kDefaultResizeBorderThickness))) {
        return Qt::SizeHorCursor;
    }
    if ((y < kDefaultResizeBorderThickness) || (y >= (h - kDefaultResizeBorderThickness))) {
        return Qt::SizeVerCursor;
    }
    return Qt::ArrowCursor;
#endif
}

Qt::Edges Utils::calculateWindowEdges(const QWindow *window, const QPoint &pos)
{
#ifdef Q_OS_MACOS
    Q_UNUSED(window);
    Q_UNUSED(pos);
    return {};
#else
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
    if (window->visibility() != QWindow::Windowed) {
        return {};
    }
    Qt::Edges edges = {};
    const int x = pos.x();
    const int y = pos.y();
    if (x < kDefaultResizeBorderThickness) {
        edges |= Qt::LeftEdge;
    }
    if (x >= (window->width() - kDefaultResizeBorderThickness)) {
        edges |= Qt::RightEdge;
    }
    if (y < kDefaultResizeBorderThickness) {
        edges |= Qt::TopEdge;
    }
    if (y >= (window->height() - kDefaultResizeBorderThickness)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
#endif
}

QString Utils::getSystemButtonIconCode(const SystemButtonType button)
{
#ifdef FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    return {};
#else // !FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
    const auto index = static_cast<int>(button);
    if (!g_fontIconsTable.contains(index)) {
        WARNING << "FIXME: Add FONT_ICON value for button" << button;
        return {};
    }
    const FONT_ICON icon = g_fontIconsTable.value(index);
#  ifdef Q_OS_WINDOWS
    // Windows 11: Segoe Fluent Icons (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-fluent-icons-font)
    // Windows 10: Segoe MDL2 Assets (https://docs.microsoft.com/en-us/windows/apps/design/style/segoe-ui-symbol-font)
    // Windows 7~8.1: Micon (http://xtoolkit.github.io/Micon/)
    if (WindowsVersionHelper::isWin10OrGreater()) {
        return QChar(icon.segoe);
    }
#  endif // Q_OS_WINDOWS
    // We always use Micon on UNIX platforms because Microsoft doesn't allow distributing
    // the Segoe icon font to other platforms than Windows.
    return QChar(icon.micon);
#endif // FRAMELESSHELPER_CORE_NO_BUNDLE_RESOURCE
}

QWindow *Utils::findWindow(const WId windowId)
{
    Q_ASSERT(windowId);
    if (!windowId) {
        return nullptr;
    }
    const QWindowList windows = QGuiApplication::topLevelWindows();
    if (windows.isEmpty()) {
        return nullptr;
    }
    for (auto &&window : qAsConst(windows)) {
        if (window && window->handle()) {
            if (window->winId() == windowId) {
                return window;
            }
        }
    }
    return nullptr;
}

void Utils::moveWindowToDesktopCenter(const GetWindowScreenCallback &getWindowScreen,
                                      const GetWindowSizeCallback &getWindowSize,
                                      const SetWindowPositionCallback &setWindowPosition,
                                      const bool considerTaskBar)
{
    Q_ASSERT(getWindowScreen);
    Q_ASSERT(getWindowSize);
    Q_ASSERT(setWindowPosition);
    if (!getWindowScreen || !getWindowSize || !setWindowPosition) {
        return;
    }
    const QSize windowSize = getWindowSize();
    if (windowSize.isEmpty() || (windowSize == kDefaultWindowSize)) {
        return;
    }
    const QScreen *screen = getWindowScreen();
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    Q_ASSERT(screen);
    if (!screen) {
        return;
    }
    const QSize screenSize = (considerTaskBar ? screen->availableVirtualSize() : screen->virtualSize());
    const QPoint offset = (considerTaskBar ? screen->availableVirtualGeometry().topLeft() : QPoint(0, 0));
    const int newX = qRound(qreal(screenSize.width() - windowSize.width()) / 2.0);
    const int newY = qRound(qreal(screenSize.height() - windowSize.height()) / 2.0);
    setWindowPosition(QPoint(newX + offset.x(), newY + offset.y()));
}

Qt::WindowState Utils::windowStatesToWindowState(const Qt::WindowStates states)
{
    if (states & Qt::WindowFullScreen) {
        return Qt::WindowFullScreen;
    }
    if (states & Qt::WindowMaximized) {
        return Qt::WindowMaximized;
    }
    if (states & Qt::WindowMinimized) {
        return Qt::WindowMinimized;
    }
    return Qt::WindowNoState;
}

bool Utils::isThemeChangeEvent(const QEvent * const event)
{
    Q_ASSERT(event);
    if (!event) {
        return false;
    }
    // QGuiApplication will only deliver theme change events to top level Q(Quick)Windows,
    // QWidgets won't get such notifications, no matter whether it's top level widget or not.
    // QEvent::ThemeChange: Send by the Windows QPA.
    // QEvent::ApplicationPaletteChange: All other platforms (Linux & macOS).
    const QEvent::Type type = event->type();
    return ((type == QEvent::ThemeChange) || (type == QEvent::ApplicationPaletteChange));
}

QColor Utils::calculateSystemButtonBackgroundColor(const SystemButtonType button, const ButtonState state)
{
    if (state == ButtonState::Unspecified) {
        return kDefaultTransparentColor;
    }
    const bool isClose = (button == SystemButtonType::Close);
    const bool isTitleColor = isTitleBarColorized();
    const bool isHovered = (state == ButtonState::Hovered);
    const auto result = [isClose, isTitleColor]() -> QColor {
        if (isClose) {
            return kDefaultSystemCloseButtonBackgroundColor;
        }
        if (isTitleColor) {
#ifdef Q_OS_WINDOWS
            return getDwmAccentColor();
#endif
#ifdef Q_OS_LINUX
            return getWmThemeColor();
#endif
#ifdef Q_OS_MACOS
            return getControlsAccentColor();
#endif
        }
        return kDefaultSystemButtonBackgroundColor;
    }();
    if (isClose) {
        return (isHovered ? result.lighter(110) : result.lighter(140));
    }
    if (!isTitleColor) {
        return (isHovered ? result.lighter(110) : result);
    }
    return (isHovered ? result.lighter(150) : result.lighter(120));
}

bool Utils::shouldAppsUseDarkMode()
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 5, 0))
    return (QGuiApplication::styleHints()->appearance() == Qt::Appearance::Dark);
#elif ((QT_VERSION >= QT_VERSION_CHECK(6, 2, 1)) && !defined(FRAMELESSHELPER_CORE_NO_PRIVATE))
    if (const QPlatformTheme * const theme = QGuiApplicationPrivate::platformTheme()) {
        return (theme->appearance() == QPlatformTheme::Appearance::Dark);
    }
    return false;
#else // ((QT_VERSION < QT_VERSION_CHECK(6, 2, 1)) || FRAMELESSHELPER_CORE_NO_PRIVATE)
#  ifdef Q_OS_WINDOWS
    return shouldAppsUseDarkMode_windows();
#  elif defined(Q_OS_LINUX)
    return shouldAppsUseDarkMode_linux();
#  elif defined(Q_OS_MACOS)
    return shouldAppsUseDarkMode_macos();
#  else
    return false;
#  endif
#endif
}

qreal Utils::roundScaleFactor(const qreal factor)
{
    Q_ASSERT(factor > 0);
    if (factor <= 0) {
        return 1;
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
#  if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    static const auto policy = QGuiApplication::highDpiScaleFactorRoundingPolicy();
    switch (policy) {
    case Qt::HighDpiScaleFactorRoundingPolicy::Unset:
#    if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        return factor;
#    else // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        return qRound(factor);
#    endif // (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    case Qt::HighDpiScaleFactorRoundingPolicy::Round:
        return qRound(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::Ceil:
        return qCeil(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::Floor:
        return qFloor(factor);
    case Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor:
        return (((factor - qreal(int(factor))) >= qreal(0.75)) ? qRound(factor) : qFloor(factor));
    case Qt::HighDpiScaleFactorRoundingPolicy::PassThrough:
        return factor;
    }
    return 1;
#  else // (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    return qRound(factor);
#  endif // (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpiScaling::roundScaleFactor(factor);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

int Utils::toNativePixels(const QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return qRound(qreal(value) * window->devicePixelRatio());
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(value, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::toNativePixels(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QPointF(QPointF(point) * window->devicePixelRatio()).toPoint();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QSize Utils::toNativePixels(const QWindow *window, const QSize &size)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QSizeF(QSizeF(size) * window->devicePixelRatio()).toSize();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(size, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QRect Utils::toNativePixels(const QWindow *window, const QRect &rect)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QRect(toNativePixels(window, rect.topLeft()), toNativePixels(window, rect.size()));
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::toNativePixels(rect, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

int Utils::fromNativePixels(const QWindow *window, const int value)
{
    Q_ASSERT(window);
    if (!window) {
        return 0;
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return qRound(qreal(value) / window->devicePixelRatio());
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(value, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QPoint Utils::fromNativePixels(const QWindow *window, const QPoint &point)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QPointF(QPointF(point) / window->devicePixelRatio()).toPoint();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(point, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QSize Utils::fromNativePixels(const QWindow *window, const QSize &size)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QSizeF(QSizeF(size) / window->devicePixelRatio()).toSize();
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(size, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

QRect Utils::fromNativePixels(const QWindow *window, const QRect &rect)
{
    Q_ASSERT(window);
    if (!window) {
        return {};
    }
#ifdef FRAMELESSHELPER_CORE_NO_PRIVATE
    return QRect(fromNativePixels(window, rect.topLeft()), fromNativePixels(window, rect.size()));
#else // !FRAMELESSHELPER_CORE_NO_PRIVATE
    return QHighDpi::fromNativePixels(rect, window);
#endif // FRAMELESSHELPER_CORE_NO_PRIVATE
}

FRAMELESSHELPER_END_NAMESPACE
