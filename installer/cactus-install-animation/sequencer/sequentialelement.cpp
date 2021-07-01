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
#include "sequentialelement.h"

#include <QRect>

struct SequentialElementPrivate {
    QList<SequencerElement*> elements;
    int doneCount = 0;
};

SequentialElement::SequentialElement(QList<SequencerElement*> elements, QObject* parent) : SequencerElement(parent) {
    d = new SequentialElementPrivate();
    d->elements = elements;
    for (SequencerElement* element : qAsConst(elements)) {
        connect(element, &SequencerElement::requestRender, this, &SequentialElement::requestRender);
        connect(element, &SequencerElement::done, this, [ = ] {
            d->doneCount++;
            if (d->elements.count() == d->doneCount) {
                emit done();
            } else {
                d->elements.at(d->doneCount)->run();
            }
        });
    }
}

SequentialElement::~SequentialElement() {
    for (SequencerElement* element : qAsConst(d->elements)) element->deleteLater();
    delete d;
}


void SequentialElement::run() {
    d->elements.first()->run();
}

void SequentialElement::render(QPainter* painter, QRect rect) {
    for (SequencerElement* element : qAsConst(d->elements)) element->render(painter, rect);
}
