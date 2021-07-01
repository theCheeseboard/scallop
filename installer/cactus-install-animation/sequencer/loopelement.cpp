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
#include "loopelement.h"

#include "installipcmanager.h"
#include <QRect>

struct LoopElementPrivate {
    int loopTimes;
    SequencerElement* toLoop;
    int looped = 0;
};

LoopElement::LoopElement(SequencerElement* toLoop, int loopTimes, QObject* parent) : SequencerElement(parent) {
    d = new LoopElementPrivate();
    d->toLoop = toLoop;
    d->loopTimes = loopTimes;

    connect(toLoop, &SequencerElement::requestRender, this, &LoopElement::requestRender);
    connect(toLoop, &SequencerElement::done, this, [ = ] {
        d->looped++;
        if (d->looped < d->loopTimes || (d->loopTimes == -1 && !InstallIpcManager::finishedSuccessfully())) {
            d->toLoop->run();
        } else {
            emit done();
        }
    });
}

LoopElement::~LoopElement() {
    d->toLoop->deleteLater();
    delete d;
}

void LoopElement::run() {
    d->looped = 0;
    d->toLoop->run();
}

void LoopElement::render(QPainter* painter, QRect rect) {
    d->toLoop->render(painter, rect);
}
