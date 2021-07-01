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
#include "parallaxobject.h"

#include <QPainter>
#include <tvariantanimation.h>

struct ParallaxObjectPrivate {
    ZoomSvgRenderer* renderer;
    double speed;
    double offset = 1;
};

ParallaxObject::ParallaxObject(ZoomSvgRenderer* renderer, double speed, QObject* parent) : QObject(parent) {
    d = new ParallaxObjectPrivate();
    d->renderer = renderer;
    d->speed = speed;

    tVariantAnimation* anim = new tVariantAnimation(this);
    anim->setStartValue(1.0);
    anim->setEndValue(-1.0);
    anim->setDuration(20000 * speed);
    anim->setEasingCurve(QEasingCurve::Linear);
    connect(anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        d->offset = value.toDouble();
        emit requestRender();
    });
    connect(anim, &tVariantAnimation::finished, this, [ = ] {
        emit done();
        anim->deleteLater();
    });
    anim->start();
}

ParallaxObject::~ParallaxObject() {
    delete d;
}

void ParallaxObject::render(QPainter* painter, QRect rect) {
    rect.moveLeft(rect.left() + rect.width() * d->offset);

    painter->save();
    painter->setOpacity(1 - d->speed);
    d->renderer->render(painter, rect);
    painter->restore();
}
