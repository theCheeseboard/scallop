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

struct AnimationStage4Private {
    double whiteFlashOpacity = 0;
    bool showBubbles = true;
    ZoomSvgRenderer* backgroundRenderer = new ZoomSvgRenderer(":/installanim/inside-hq-night-emergency.svg");
    ZoomSvgRenderer* backgroundRendererBubbles = new ZoomSvgRenderer(":/installanim/inside-hq-night-emergency-bubbles.svg");
    Sequencer* sequencer;

    QPoint renderOffset;
    double renderOffsetHeightPerc = 0;
    double coverOffset = 1;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage4)

void AnimationStage4::start() {

    d->sequencer = new Sequencer(this);

    d->sequencer->addElement({
        new ParallelElement({
            new SequentialElement({
                new FunctionElement([ = ] {d->renderOffset = QPoint(-50, 50);}),
                new PauseElement(50),
                new FunctionElement([ = ] {d->renderOffset = QPoint(50, 50);}),
                new PauseElement(50),
                new FunctionElement([ = ] {d->renderOffset = QPoint(50, -50);}),
                new PauseElement(50),
                new FunctionElement([ = ] {d->renderOffset = QPoint(-50, 50);}),
                new PauseElement(50),
                new FunctionElement([ = ] {d->renderOffset = QPoint(-50, -50);}),
                new PauseElement(50),
                new FunctionElement([ = ] {d->renderOffset = QPoint(0, 0);}),
                new TextBoxElement(new TextBox({
                    tr("[CHOSEN TEMPLE FOUND!]"),
                    tr("[INSTALL THE %1 CUBE IN THE TEMPLE]").arg(InstallerData::systemName()).toUpper(),
                    tr("[DO THIS, AND YOU WILL BE SHOWERED WITH RICHES]")
                }, TextBox::RobotCharacter)),
            }),
            new AnimationElement(1.0, 0.0, 500, [ = ](QVariant value) {
                d->whiteFlashOpacity = value.toDouble();
            })
        }),
        new FunctionElement([ = ] {
            d->showBubbles = false;
        }),
        new TextBoxElement(new TextBox({tr("Never mind then. Let's hit the road!")})),
        new ParallelElement({
            new AnimationElement(0.0, -0.5, 1000, [ = ](QVariant value) {
                d->renderOffsetHeightPerc = value.toDouble();
            }, QEasingCurve::InCubic),
            new AnimationElement(1.0, 0.0, 1000, [ = ](QVariant value) {
                d->coverOffset = value.toDouble();
            }, QEasingCurve::InCubic)
        }),
        new PauseElement(500)
    });

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage4::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::black);
    painter->setPen(Qt::white);

    QRect backgroundRect = renderRect;
    backgroundRect.setTopLeft(backgroundRect.topLeft() + d->renderOffset);
    backgroundRect.moveTop(backgroundRect.top() + size.height() * d->renderOffsetHeightPerc);

    if (d->showBubbles) {
        d->backgroundRendererBubbles->render(painter, backgroundRect);
    } else {
        d->backgroundRenderer->render(painter, backgroundRect);
    }

    d->sequencer->render(painter, renderRect);

    painter->setOpacity(d->whiteFlashOpacity);
    painter->fillRect(renderRect, Qt::white);

    painter->setOpacity(1);
    QRect cover = renderRect;
    cover.moveTop(size.height() * d->coverOffset);
    painter->fillRect(cover, Qt::black);
}
