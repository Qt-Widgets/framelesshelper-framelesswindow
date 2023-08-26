/*
 * MIT License
 *
 * Copyright (C) 2021-2023 by wangwenx190 (Yuhang Zhao)
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

#include "framelessmainwindow.h"
#include "framelessmainwindow_p.h"
#include "framelesswidgetshelper.h"
#include "widgetssharedhelper_p.h"
#include <FramelessHelper/Core/utils.h>
#include <QtCore/qloggingcategory.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

[[maybe_unused]] static Q_LOGGING_CATEGORY(lcFramelessMainWindow, "wangwenx190.framelesshelper.widgets.framelessmainwindow")

#ifdef FRAMELESSHELPER_WIDGETS_NO_DEBUG_OUTPUT
#  define INFO QT_NO_QDEBUG_MACRO()
#  define DEBUG QT_NO_QDEBUG_MACRO()
#  define WARNING QT_NO_QDEBUG_MACRO()
#  define CRITICAL QT_NO_QDEBUG_MACRO()
#else
#  define INFO qCInfo(lcFramelessMainWindow)
#  define DEBUG qCDebug(lcFramelessMainWindow)
#  define WARNING qCWarning(lcFramelessMainWindow)
#  define CRITICAL qCCritical(lcFramelessMainWindow)
#endif

using namespace Global;

FramelessMainWindowPrivate::FramelessMainWindowPrivate(FramelessMainWindow *q) : QObject(q)
{
    Q_ASSERT(q);
    if (!q) {
        return;
    }
    q_ptr = q;
}

FramelessMainWindowPrivate::~FramelessMainWindowPrivate() = default;

FramelessMainWindowPrivate *FramelessMainWindowPrivate::get(FramelessMainWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

const FramelessMainWindowPrivate *FramelessMainWindowPrivate::get(const FramelessMainWindow *pub)
{
    Q_ASSERT(pub);
    if (!pub) {
        return nullptr;
    }
    return pub->d_func();
}

FramelessMainWindow::FramelessMainWindow(QWidget *parent, const Qt::WindowFlags flags)
    : QMainWindow(parent, flags), d_ptr(new FramelessMainWindowPrivate(this))
{
    FramelessWidgetsHelper::get(this)->extendsContentIntoTitleBar();
    Q_D(FramelessMainWindow);
    d->sharedHelper = new WidgetsSharedHelper(d);
    d->sharedHelper->setup(this);
}

FramelessMainWindow::~FramelessMainWindow() = default;

bool FramelessMainWindow::isNormal() const
{
    return (Utils::windowStatesToWindowState(windowState()) == Qt::WindowNoState);
}

bool FramelessMainWindow::isZoomed() const
{
    return (isMaximized() || isFullScreen());
}

void FramelessMainWindow::toggleMaximized()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void FramelessMainWindow::toggleFullScreen()
{
    Q_D(FramelessMainWindow);
    if (isFullScreen()) {
        setWindowState(d->savedWindowState);
    } else {
        d->savedWindowState = Utils::windowStatesToWindowState(windowState());
        showFullScreen();
    }
}

FRAMELESSHELPER_END_NAMESPACE
