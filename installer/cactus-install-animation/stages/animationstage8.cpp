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

struct AnimationStage8Private {
    Sequencer* sequencer;
    double opacity = 0;
    int textOffset = 0;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage8)

void AnimationStage8::start() {
    d->sequencer = new Sequencer(this);

    d->sequencer->addElement({
        new ParallelElement({
            new AnimationElement(0.0, 1.0, 3000, [ = ](QVariant value) {
                d->opacity = value.toDouble();
            }, QEasingCurve::InCubic),
            new AnimationElement(100, 0, 3000, [ = ](QVariant value) {
                d->textOffset = value.toInt();
            }, QEasingCurve::OutCubic)
        }),
        new PauseElement(300)
    });

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage8::render(QPainter* painter, QSize size) {
    QRect renderRect(0, 0, size.width(), size.height());

    painter->fillRect(renderRect, Qt::white);

    QFont font("JetBrains Mono");
    font.setPixelSize(size.height() / 20);
    painter->setFont(font);

    QFontMetrics metrics(font);

    painter->setPen(Qt::black);
    painter->setOpacity(d->opacity);

    QRect textRect;
    textRect.setWidth(size.width());
    textRect.setHeight(metrics.height());
    textRect.moveCenter(renderRect.center());
    textRect.moveBottom(renderRect.center().y());

    textRect.moveTop(textRect.top() + d->textOffset);

    painter->drawText(textRect, Qt::AlignCenter, tr("Welcome to %1!").arg(InstallerData::systemName()));
}
