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
#include "animationstages.h"

#include "installerdata.h"
#include "../zoomsvgrenderer.h"
#include "../sequencer.h"
#include "../textbox.h"
#include <tpromise.h>
#include <QRandomGenerator>
#include "../parallaxobject.h"

struct AnimationStage6Private {
    double whiteFlashOpacity = 0;
    Sequencer* sequencer;

    QList<ParallaxObject*> parallaxObjects;

    ZoomSvgRenderer* bgRenderer, *carRenderer, *roadRenderer, *oldBgRenderer, *oldCarRenderer, *oldRoadRenderer;
    bool transitioning = false;
    double transitionProgress = 0;

    QPoint renderOffset;
    double coverOffset = 1;
    double carOffset = 0;
    int carTopOffset = 0;

    ZoomSvgRenderer* outdoorNight = new ZoomSvgRenderer(":/installanim/outdoors-night.svg");
    ZoomSvgRenderer* outdoorDay = new ZoomSvgRenderer(":/installanim/outdoors-day.svg");
    ZoomSvgRenderer* carNight = new ZoomSvgRenderer(":/installanim/car-night.svg");
    ZoomSvgRenderer* roadNight = new ZoomSvgRenderer(":/installanim/road-night.svg");

    double speed = 0;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage6)

#include <tlogger.h>

void AnimationStage6::start() {
    d->sequencer = new Sequencer(this);

    d->bgRenderer = d->outdoorNight;
    d->carRenderer = d->carNight;
    d->roadRenderer = d->roadNight;

    auto generateElement = [ = ](QString element) {
        ParallaxObject* obj = new ParallaxObject(new ZoomSvgRenderer(QStringLiteral(":/installanim/%1.svg").arg(element)), d->speed);
        connect(obj, &ParallaxObject::requestRender, this, &CactusAnimationStage::requestRender);
        connect(obj, &ParallaxObject::done, this, [ = ] {
            d->parallaxObjects.removeOne(obj);
            obj->deleteLater();
        });
        d->parallaxObjects.append(obj);
    };
    auto changeEnvironment = [ = ](ZoomSvgRenderer * background, ZoomSvgRenderer * car, ZoomSvgRenderer * road) {
        return new SequentialElement({
            new FunctionElement([ = ] {
                if (d->transitioning) return;
                d->oldBgRenderer = d->bgRenderer;
                d->oldCarRenderer = d->carRenderer;
                d->oldRoadRenderer = d->roadRenderer;
                d->bgRenderer = background;
                d->carRenderer = car;
                d->roadRenderer = road;
                d->transitioning = true;
            }),
            new AnimationElement(1.0, 0.0, 2000, [ = ](QVariant value) {
                d->transitionProgress = value.toDouble();
            }),
            new FunctionElement([ = ] {
                d->transitioning = false;
            })
        });
    };

    d->sequencer->addElement(new SequentialElement({
        new LoopElement(new SequentialElement({
            new FunctionElement([ = ] {
                d->speed = QRandomGenerator::system()->bounded(1.0);
            }),
            //Oneshot element: don't wait for this one to finish
            new FunctionElement([ = ] {
                RandomElement* el = new RandomElement({
                    new FunctionElement(std::bind(generateElement, "object-building")),
                    new FunctionElement(std::bind(generateElement, "object-building")),
                    new FunctionElement(std::bind(generateElement, "object-building2")),
                    new FunctionElement(std::bind(generateElement, "object-building2")),
                    new FunctionElement(std::bind(generateElement, "object-tower")),
                    new FunctionElement(std::bind(generateElement, "object-tower")),
                    changeEnvironment(d->outdoorNight, d->carNight, d->roadNight),
                    changeEnvironment(d->outdoorDay, d->carNight, d->roadNight),
                    new SequentialElement({
                        new AnimationElement(0, -30, 200, [ = ](QVariant value) {
                            d->carTopOffset = value.toInt();
                        }, QEasingCurve::OutCubic),
                        new AnimationElement(-30, 0, 200, [ = ](QVariant value) {
                            d->carTopOffset = value.toInt();
                        }, QEasingCurve::InCubic)
                    })
                });
                connect(el, &RandomElement::requestRender, this, &CactusAnimationStage::requestRender);
                connect(el, &RandomElement::done, el, &RandomElement::deleteLater);
                el->run();
            }),
            new PauseElement(-3000)
        }), -1),
        new AnimationElement(1.0, 0.0, 500, [ = ](QVariant value) {
            d->coverOffset = value.toDouble();
        })
    }));

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage6::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::black);
    painter->setPen(Qt::white);

    QRect backgroundRect = renderRect;
    backgroundRect.setTopLeft(backgroundRect.topLeft() + d->renderOffset);

    d->bgRenderer->render(painter, backgroundRect);
    if (d->transitioning) {
        painter->save();
        painter->setOpacity(d->transitionProgress);
        d->oldBgRenderer->render(painter, backgroundRect);
        painter->restore();
    }

    for (ParallaxObject* object : d->parallaxObjects) {
        object->render(painter, backgroundRect);
    }

    QRect carRect = renderRect;
    carRect.moveTop(carRect.top() + d->carTopOffset);
    carRect.moveLeft(carRect.left() + size.width() * d->carOffset);
    d->carRenderer->render(painter, carRect);
    if (d->transitioning) {
        painter->save();
        painter->setOpacity(d->transitionProgress);
        d->oldCarRenderer->render(painter, backgroundRect);
        painter->restore();
    }

    d->roadRenderer->render(painter, backgroundRect);
    if (d->transitioning) {
        painter->save();
        painter->setOpacity(d->transitionProgress);
        d->oldRoadRenderer->render(painter, backgroundRect);
        painter->restore();
    }

    d->sequencer->render(painter, renderRect);

    painter->setOpacity(d->whiteFlashOpacity);
    painter->fillRect(renderRect, Qt::white);

    painter->setOpacity(1);
    QRect cover = renderRect;
    cover.moveLeft(size.width() * d->coverOffset);
    painter->fillRect(cover, Qt::black);
}
