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
#include "randomelement.h"

#include <QRandomGenerator>
#include <QRect>

struct RandomElementPrivate {
    QList<SequencerElement*> elements;
};

RandomElement::RandomElement(QList<SequencerElement*> elements, QObject* parent) : SequencerElement(parent) {
    d = new RandomElementPrivate();
    d->elements = elements;
    for (SequencerElement* element : qAsConst(elements)) {
        connect(element, &SequencerElement::requestRender, this, &SequencerElement::requestRender);
        connect(element, &SequencerElement::done, this, &RandomElement::done);
    }
}

RandomElement::~RandomElement() {
    for (SequencerElement* element : qAsConst(d->elements)) element->deleteLater();
    delete d;
}

void RandomElement::run() {
    SequencerElement* selected = d->elements.at(QRandomGenerator::system()->bounded(d->elements.length()));
    selected->run();
}

void RandomElement::render(QPainter* painter, QRect rect) {
    for (SequencerElement* element : qAsConst(d->elements)) element->render(painter, rect);
}
