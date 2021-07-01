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

struct AnimationStage7Private {
    Sequencer* sequencer;

    ZoomSvgRenderer* backgroundRenderer = new ZoomSvgRenderer(":/installanim/temple.svg");
    ZoomSvgRenderer* coverRenderer = new ZoomSvgRenderer(":/installanim/temple-reveal.svg");

    ZoomSvgRenderer* cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-at-temple-1.svg");

    int backgroundOffset = 300;
    double coverOffset = 0;
    double cactusOffset = -1;
    double whiteCoverOpacity = 0;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage7)

void AnimationStage7::start() {
    d->sequencer = new Sequencer(this);

    d->sequencer->addElement({
        //TODO: crash sound effect
        new SoundElement("qrc:/installanim/car-crash.wav"),
        new TextBoxElement(new TextBox({tr("ARE YOU OKAY???")})),
        new TextBoxElement(new TextBox({tr("I'm fine. Where are we?")})),
        new ParallelElement({
            new AnimationElement(0.0, -2.0, 4000, [ = ](QVariant value) {
                d->coverOffset = value.toDouble();
            }),
            new AnimationElement(300, 0, 3000, [ = ](QVariant value) {
                d->backgroundOffset = value.toInt();
            })
        }),
        new TextBoxElement(new TextBox({tr("[PLEASE INSTALL THE %1 CUBE]").arg(InstallerData::systemName()).toUpper()}, TextBox::RobotCharacter)),
        new PauseElement(500),
        new ParallelElement({
            new TextBoxElement(new TextBox({tr("Quick, the %1 cube!").arg(InstallerData::systemName())})),
            new AnimationElement(-1.0, 0.0, 500, [ = ](QVariant value) {
                d->cactusOffset = value.toDouble();
            })
        }),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-at-temple-2.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-1.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-2.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-3.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-4.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-5.svg");
        }),
        new PauseElement(300),
        new FunctionElement([ = ] {
            d->cactusRenderer->deleteLater();
            d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-6.svg");
        }),
        new PauseElement(1000),
        new TextBoxElement(new TextBox({tr("Did we do it??")})),
        new ParallelElement({
            new LoopElement(new SequentialElement({
                new FunctionElement([ = ] {
                    d->cactusRenderer->deleteLater();
                    d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-7.svg");
                }),
                new PauseElement(300),
                new FunctionElement([ = ] {
                    d->cactusRenderer->deleteLater();
                    d->cactusRenderer = new ZoomSvgRenderer(":/installanim/cactus-man-throw-8.svg");
                }),
                new PauseElement(300)
            }), 3),
            new AnimationElement(0.0, 1.0, 2000, [ = ](QVariant value) {
                d->whiteCoverOpacity = value.toDouble();
            }),
            new TextBoxElement(new TextBox({tr("[%1 CUBE ACCEPTED]").arg(InstallerData::systemName()).toUpper()}, TextBox::RobotCharacter)),
        })
    });

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage7::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::black);

    QRect backgroundRect = renderRect;
    backgroundRect.setWidth(backgroundRect.width() * 1.1);
    backgroundRect.moveRight(size.width() + d->backgroundOffset);
    d->backgroundRenderer->render(painter, backgroundRect);

    QRect cactusRect = renderRect;
    cactusRect.moveLeft(cactusRect.left() + size.width() * d->cactusOffset);
    d->cactusRenderer->render(painter, cactusRect);

    QRect cover = renderRect;
    cover.moveLeft(size.width() * d->coverOffset);
    painter->fillRect(cover, Qt::black);
    cover.moveLeft(cover.right());
    d->coverRenderer->render(painter, cover);

    painter->setOpacity(d->whiteCoverOpacity);
    painter->fillRect(renderRect, Qt::white);

    painter->setOpacity(1);
    d->sequencer->render(painter, renderRect);
}
