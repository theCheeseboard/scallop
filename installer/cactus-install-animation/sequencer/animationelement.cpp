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
#include "animationelement.h"

#include <tvariantanimation.h>

struct AnimationElementPrivate {
    tVariantAnimation* anim;
};

AnimationElement::AnimationElement(QVariant start, QVariant end, int duration, std::function<void (QVariant)> changeCallback, QEasingCurve easing, QObject* parent) : SequencerElement(parent) {
    d = new AnimationElementPrivate();
    d->anim = new tVariantAnimation(this);
    d->anim->setStartValue(start);
    d->anim->setEndValue(end);
    d->anim->setDuration(duration);
    d->anim->setEasingCurve(easing);
    connect(d->anim, &tVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        changeCallback(value);
        requestRender();
    });
    connect(d->anim, &tVariantAnimation::finished, this, &AnimationElement::done);
}

AnimationElement::~AnimationElement() {
    delete d;
}

void AnimationElement::run() {
    d->anim->start();
}
