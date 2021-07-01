/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2021 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "zoomsvgrenderer.h"

#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

struct ZoomSvgRendererPrivate {
    QSvgRenderer* renderer;
    Qt::AspectRatioMode mode = Qt::KeepAspectRatioByExpanding;
};

ZoomSvgRenderer::ZoomSvgRenderer(QString file, QObject* parent) : QObject(parent) {
    d = new ZoomSvgRendererPrivate();
    d->renderer = new QSvgRenderer(file, this);
}

ZoomSvgRenderer::~ZoomSvgRenderer() {
    d->renderer->deleteLater();
    delete d;
}

void ZoomSvgRenderer::setMode(Qt::AspectRatioMode mode) {
    d->mode = mode;
}

QSize ZoomSvgRenderer::size() {
    return d->renderer->defaultSize();
}

QRect ZoomSvgRenderer::bounds(QString id, QRect rect) {
    double factor = QSizeF(d->renderer->defaultSize().scaled(rect.size(), d->mode)).width() / QSizeF(d->renderer->defaultSize()).width() * 3.8;

    QRectF newRect = d->renderer->transformForElement(id).mapRect(d->renderer->boundsOnElement(id).toRect());
    newRect.moveTopLeft(newRect.topLeft() * factor);
    newRect.setSize(newRect.size() * factor);
    newRect.moveTopLeft(newRect.topLeft() + rect.topLeft());

    return newRect.toRect();
}

void ZoomSvgRenderer::render(QPainter* painter, QRect rect) {
    QPixmap px(rect.size());
    px.fill(Qt::transparent);

    QPainter pxPainter(&px);

    QRect drawRect;
    drawRect.setSize(d->renderer->defaultSize().scaled(rect.size(), d->mode));
    drawRect.moveCenter(QRect(QPoint(0, 0), px.size()).center());
    d->renderer->render(&pxPainter, drawRect);

    pxPainter.end();
    painter->drawPixmap(rect, px);
}
