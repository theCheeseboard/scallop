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
#include "sequencer.h"

#include <QRect>
#include "sequencer/sequencerelement.h"

struct SequencerPrivate {
    QList<SequencerElement*> elements;
    QList<SequencerElement*>::Iterator current;
};

Sequencer::Sequencer(QObject* parent) : QObject(parent) {
    d = new SequencerPrivate();
}

Sequencer::~Sequencer() {
    for (SequencerElement* element : qAsConst(d->elements)) element->deleteLater();
    delete d;
}

void Sequencer::addElement(SequencerElement* element) {
    addElement(QList<SequencerElement*>({element}));
}

void Sequencer::addElement(QList<SequencerElement*> elements) {
    for (SequencerElement* element : elements) {
        connect(element, &SequencerElement::done, this, [ = ] {
            d->current++;
            if (d->current == d->elements.end()) {
                emit done();
            } else {
                emit requestRender();
                (*d->current)->run();
            }
        });
        connect(element, &SequencerElement::requestRender, this, &Sequencer::requestRender);
        d->elements.append(element);
    }
}

void Sequencer::start() {
    d->current = d->elements.begin();
    (*d->current)->run();
}

void Sequencer::render(QPainter* painter, QRect rect) {
    for (SequencerElement* element : qAsConst(d->elements)) element->render(painter, rect);
}
