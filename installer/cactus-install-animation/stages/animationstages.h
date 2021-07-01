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
#ifndef ANIMATIONSTAGES_H
#define ANIMATIONSTAGES_H

#include <tvariantanimation.h>
#include "../cactusanimationstage.h"

#define NEW_ANIMATION_STAGE(name) \
    struct name##Private; \
    class name : public CactusAnimationStage { \
            Q_OBJECT \
        public: \
            explicit name(QObject *parent = nullptr); \
            ~name(); \
            void start(); \
            void render(QPainter*painter, QSize size); \
        private: \
            name##Private *d; \
    };

#define ANIMATION_STAGE_BOILERPLATE(name) \
    name::name(QObject* parent) : CactusAnimationStage(parent) { \
        d = new name##Private; \
    } \
    name::~name() {\
        delete d; \
    }

//#define ANIMATION_INTERP(from, to, duration, changeCallback, then) { \
//        tVariantAnimation* anim = new tVariantAnimation(); \
//        anim->setStartValue(from); \
//        anim->setEndValue(to); \
//        anim->setDuration(duration); \
//        anim->setEasingCurve(QEasingCurve::OutCubic); \
//        QObject::connect(anim, &tVariantAnimation::valueChanged, this, [=](QVariant value) {changeCallback}); \
//        QObject::connect(anim, &tVariantAnimation::finished, this, [=] { \
//                anim->deleteLater(); \
//                then; \
//            }); \
//        anim->start(); \
//    }

//#define ANIMATION_INTERP_BLOCKING(from, to, duration, changeCallback) { \
//        QEventLoop* loop = new QEventLoop(); \
//        ANIMATION_INTERP(from, to, duration, changeCallback, loop->exit()) \
//        loop->exec(); \
//        loop->deleteLater(); \
//    }

NEW_ANIMATION_STAGE(AnimationStage1)
NEW_ANIMATION_STAGE(AnimationStage2)
NEW_ANIMATION_STAGE(AnimationStage3)
NEW_ANIMATION_STAGE(AnimationStage4)
NEW_ANIMATION_STAGE(AnimationStage5)

#endif // ANIMATIONSTAGES_H
