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
#include <tpromise.h>
#include "../sequencer.h"
#include "../zoomsvgrenderer.h"

struct AnimationStage1Private {
    QString line1Text;
    QString line2Text;
    double opacity = 1;
    Sequencer* sequencer;

    double logoOpacity = 1;
};

ANIMATION_STAGE_BOILERPLATE(AnimationStage1)

void AnimationStage1::start() {
    d->sequencer = new Sequencer(this);

    d->sequencer->addElement(new AnimationElement(1.0, 0.0, 500, [ = ](QVariant value) {
        d->logoOpacity = value.toDouble();
    }));

    for (QChar c : QLocale().toString(QDate::currentDate())) {
        d->sequencer->addElement({
            new FunctionElement([ = ] {
                d->line1Text += c;
            }),
            new PauseElement(50)
        });
    }
    d->sequencer->addElement(new PauseElement(500));

    for (QChar c : tr("%1 HQ", "HQ for Headquarters").arg(InstallerData::systemName())) {
        d->sequencer->addElement({
            new FunctionElement([ = ] {
                d->line2Text += c;
            }),
            new PauseElement(50)
        });
    }

    d->sequencer->addElement(new PauseElement(2000));
    d->sequencer->addElement(new AnimationElement(1.0, 0.0, 500, [ = ](QVariant value) {
        d->opacity = value.toDouble();
    }));

    connect(d->sequencer, &Sequencer::requestRender, this, &CactusAnimationStage::requestRender);
    connect(d->sequencer, &Sequencer::done, this, &CactusAnimationStage::stageComplete);
    d->sequencer->start();
}

void AnimationStage1::render(QPainter* painter, QSize size) {
    QFont font("JetBrains Mono");
    font.setPixelSize(size.height() / 20);
    painter->setFont(font);

    QRect renderRect(0, 0, size.width(), size.height());

    QFontMetrics metrics(font);

    painter->fillRect(renderRect, Qt::black);
    painter->setPen(Qt::white);
    painter->setOpacity(d->opacity);

    QRect textRect;
    textRect.setWidth(size.width());
    textRect.setHeight(metrics.height());
    textRect.moveCenter(renderRect.center());
    textRect.moveBottom(renderRect.center().y());

    painter->drawText(textRect, Qt::AlignCenter, d->line1Text);
    textRect.moveTop(renderRect.center().y());
    painter->drawText(textRect, Qt::AlignCenter, d->line2Text);

    painter->setOpacity(d->logoOpacity);

    QSize systemIconSize = InstallerData::systemIconSize();
    systemIconSize.scale(qMin(size.width(), SC_DPI(300)), qMin(size.height(), SC_DPI(300)), Qt::KeepAspectRatio);

    QRect systemIconRect;
    systemIconRect.setSize(systemIconSize);
    systemIconRect.moveCenter(renderRect.center());
    painter->drawPixmap(systemIconRect, InstallerData::systemIcon(systemIconSize));
}
