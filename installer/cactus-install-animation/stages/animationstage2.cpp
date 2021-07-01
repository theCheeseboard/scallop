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

struct AnimationStage2Private {
    double backgroundOpacity = 0;
    double whiteFlashOpacity = 0;
    ZoomSvgRenderer* backgroundRenderer = new ZoomSvgRenderer(":/installanim/hq-at-night.svg");
    Sequencer* sequencer;

    QPoint renderOffset;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage2)

void AnimationStage2::start() {

    d->sequencer = new Sequencer(this);

    d->sequencer->addElement({
        new AnimationElement(0.0, 1.0, 500, [ = ](QVariant value) {
            d->backgroundOpacity = value.toDouble();
        }),
        new PauseElement(500),
        new TextBoxElement(new TextBox(tr("...so do you think any more people will install %1 today?").arg(InstallerData::systemName()))),
        new PauseElement(100),
        new AnimationElement(1.0, 0.0, 500, [ = ](QVariant value) {
            d->backgroundOpacity = value.toDouble();
        })
    });

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage2::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::black);
    painter->setPen(Qt::white);
    painter->setOpacity(d->backgroundOpacity);

    QRect backgroundRect = renderRect;
    backgroundRect.setTopLeft(backgroundRect.topLeft() + d->renderOffset);
    d->backgroundRenderer->render(painter, backgroundRect);

    d->sequencer->render(painter, renderRect);

    painter->setOpacity(d->whiteFlashOpacity);
    painter->fillRect(renderRect, Qt::white);
}
