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

#include "widgetssharedhelper_p.h"
#include <QtCore/qcoreevent.h>
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtGui/qwindow.h>
#include <QtWidgets/qwidget.h>
#include <micamaterial.h>
#include <micamaterial_p.h>
#include <utils.h>

FRAMELESSHELPER_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcWidgetsSharedHelper, "wangwenx190.framelesshelper.widgets.widgetssharedhelper")
#define INFO qCInfo(lcWidgetsSharedHelper)
#define DEBUG qCDebug(lcWidgetsSharedHelper)
#define WARNING qCWarning(lcWidgetsSharedHelper)
#define CRITICAL qCCritical(lcWidgetsSharedHelper)

using namespace Global;

WidgetsSharedHelper::WidgetsSharedHelper(QObject *parent) : QObject(parent)
{
}

WidgetsSharedHelper::~WidgetsSharedHelper() = default;

void WidgetsSharedHelper::setup(QWidget *widget)
{
    Q_ASSERT(widget);
    if (!widget) {
        return;
    }
    if (m_targetWidget == widget) {
        return;
    }
    m_targetWidget = widget;
    m_micaMaterial = MicaMaterial::attach(m_targetWidget);
    if (m_micaRedrawConnection) {
        disconnect(m_micaRedrawConnection);
        m_micaRedrawConnection = {};
    }
    m_micaRedrawConnection = connect(m_micaMaterial, &MicaMaterial::shouldRedraw,
        this, [this](){
            if (m_targetWidget) {
                m_targetWidget->update();
            }
        });
    m_targetWidget->installEventFilter(this);
    updateContentsMargins();
    m_targetWidget->update();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QScreen *screen = m_targetWidget->screen();
#else
    QScreen *screen = m_targetWidget->windowHandle()->screen();
#endif
    handleScreenChanged(screen);
    connect(m_targetWidget->windowHandle(), &QWindow::screenChanged, this, &WidgetsSharedHelper::handleScreenChanged);
}

bool WidgetsSharedHelper::isMicaEnabled() const
{
    return m_micaEnabled;
}

void WidgetsSharedHelper::setMicaEnabled(const bool value)
{
    if (m_micaEnabled == value) {
        return;
    }
    m_micaEnabled = value;
    if (m_targetWidget) {
        m_targetWidget->update();
    }
    Q_EMIT micaEnabledChanged();
}

bool WidgetsSharedHelper::eventFilter(QObject *object, QEvent *event)
{
    Q_ASSERT(object);
    Q_ASSERT(event);
    if (!object || !event) {
        return false;
    }
    if (!m_targetWidget) {
        return QObject::eventFilter(object, event);
    }
    if (!object->isWidgetType()) {
        return QObject::eventFilter(object, event);
    }
    const auto widget = qobject_cast<QWidget *>(object);
    if (widget != m_targetWidget) {
        return QObject::eventFilter(object, event);
    }
    switch (event->type()) {
    case QEvent::Paint: {
        const auto paintEvent = static_cast<QPaintEvent *>(event);
        paintEventHandler(paintEvent);
    } break;
    case QEvent::WindowStateChange: {
        changeEventHandler(event);
    } break;
    case QEvent::Move:
    case QEvent::Resize: {
        if (m_micaEnabled) {
            m_targetWidget->update();
        }
    } break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void WidgetsSharedHelper::changeEventHandler(QEvent *event)
{
    Q_ASSERT(event);
    if (!event) {
        return;
    }
    if (event->type() != QEvent::WindowStateChange) {
        return;
    }
    updateContentsMargins();
    QMetaObject::invokeMethod(m_targetWidget, "hiddenChanged");
    QMetaObject::invokeMethod(m_targetWidget, "normalChanged");
    QMetaObject::invokeMethod(m_targetWidget, "zoomedChanged");
}

void WidgetsSharedHelper::paintEventHandler(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (m_micaEnabled && m_micaMaterial) {
        QPainter painter(m_targetWidget);
        m_micaMaterial->paint(&painter, m_targetWidget->size(),
            m_targetWidget->mapToGlobal(QPoint(0, 0)));
    }
#ifdef Q_OS_WINDOWS
    if (shouldDrawFrameBorder()) {
        QPainter painter(m_targetWidget);
        painter.save();
        QPen pen = {};
        pen.setColor(Utils::getFrameBorderColor(m_targetWidget->isActiveWindow()));
        pen.setWidth(kDefaultWindowFrameBorderThickness);
        painter.setPen(pen);
        // In fact, we should use "m_targetWidget->width() - 1" here but we can't
        // because Qt's drawing system has some rounding errors internally and if
        // we minus one here we'll get a one pixel gap, so sad. But drawing a line
        // with a little extra pixels won't hurt anyway.
        painter.drawLine(0, 0, m_targetWidget->width(), 0);
        painter.restore();
    }
#endif
}

bool WidgetsSharedHelper::shouldDrawFrameBorder() const
{
#ifdef Q_OS_WINDOWS
    static const bool isWin11OrGreater = Utils::isWindowsVersionOrGreater(WindowsVersion::_11_21H2);
    return (Utils::isWindowFrameBorderVisible() && !isWin11OrGreater
            && (Utils::windowStatesToWindowState(m_targetWidget->windowState()) == Qt::WindowNoState));
#else
    return false;
#endif
}

void WidgetsSharedHelper::handleScreenChanged(QScreen *screen)
{
    Q_ASSERT(m_targetWidget);
    if (!m_targetWidget) {
        return;
    }
    // The QScreen handle can be null if a window was moved out of a screen.
    if (!screen) {
        return;
    }
    if (m_screen == screen) {
        return;
    }
    m_screen = screen;
    m_screenDpr = m_screen->devicePixelRatio();
    if (m_screenDpiChangeConnection) {
        disconnect(m_screenDpiChangeConnection);
        m_screenDpiChangeConnection = {};
    }
    m_screenDpiChangeConnection = connect(m_screen, &QScreen::physicalDotsPerInchChanged,
        this, [this](const qreal dpi){
            Q_UNUSED(dpi);
            const qreal currentDpr = m_screen->devicePixelRatio();
            if (m_screenDpr == currentDpr) {
                return;
            }
            m_screenDpr = currentDpr;
            if (m_micaEnabled && m_micaMaterial) {
                MicaMaterialPrivate::get(m_micaMaterial)->maybeGenerateBlurredWallpaper(true);
            }
        });
}

void WidgetsSharedHelper::updateContentsMargins()
{
#ifdef Q_OS_WINDOWS
    m_targetWidget->setContentsMargins(0, (shouldDrawFrameBorder() ? kDefaultWindowFrameBorderThickness : 0), 0, 0);
#endif
}

FRAMELESSHELPER_END_NAMESPACE
