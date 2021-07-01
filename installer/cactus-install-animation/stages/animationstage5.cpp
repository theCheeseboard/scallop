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

struct AnimationStage5Private {
    double whiteFlashOpacity = 0;
    ZoomSvgRenderer* backgroundRenderer = new ZoomSvgRenderer(":/installanim/outdoors-night.svg");
    ZoomSvgRenderer* carRenderer = new ZoomSvgRenderer(":/installanim/car-night.svg");
    ZoomSvgRenderer* roadRenderer = new ZoomSvgRenderer(":/installanim/road-night.svg");
    Sequencer* sequencer;

    QPoint renderOffset;
    double coverOffset = 0;
    double carOffset;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage5)

void AnimationStage5::start() {
    d->sequencer = new Sequencer(this);

    d->sequencer->addElement({
        new ParallelElement({
            new AnimationElement(0.0, -1.0, 1000, [ = ](QVariant value) {
                d->coverOffset = value.toDouble();
            }),
            new AnimationElement(-0.2, 0.0, 1000, [ = ](QVariant value) {
                d->carOffset = value.toDouble();
            })
        })
    });

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage5::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::black);
    painter->setPen(Qt::white);

    QRect backgroundRect = renderRect;
    backgroundRect.setTopLeft(backgroundRect.topLeft() + d->renderOffset);

    d->backgroundRenderer->render(painter, backgroundRect);

    QRect carRect = renderRect;
    carRect.moveLeft(carRect.left() + size.width() * d->carOffset);
    d->carRenderer->render(painter, carRect);

    d->roadRenderer->render(painter, backgroundRect);

    d->sequencer->render(painter, renderRect);

    painter->setOpacity(d->whiteFlashOpacity);
    painter->fillRect(renderRect, Qt::white);

    painter->setOpacity(1);
    QRect cover = renderRect;
    cover.moveLeft(size.width() * d->coverOffset);
    painter->fillRect(cover, Qt::black);
}
