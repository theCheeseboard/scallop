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
#include "oneshotelement.h"

#include <QRect>

struct OneshotElementPrivate {
    SequencerElement* element;
};

OneshotElement::OneshotElement(SequencerElement* element, QObject* parent) : SequencerElement(parent) {
    d = new OneshotElementPrivate();
    d->element = element;
    connect(d->element, &SequencerElement::requestRender, this, &OneshotElement::requestRender);
}

OneshotElement::~OneshotElement() {
    d->element->deleteLater();
    delete d;
}


void OneshotElement::run() {
    d->element->run();
    emit done();
}

void OneshotElement::render(QPainter* painter, QRect rect) {
    d->element->render(painter, rect);
}
