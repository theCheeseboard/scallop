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
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <QObject>

#include "sequencer/functionelement.h"
#include "sequencer/pauseelement.h"
#include "sequencer/animationelement.h"
#include "sequencer/textboxelement.h"
#include "sequencer/parallelelement.h"
#include "sequencer/sequentialelement.h"
#include "sequencer/loopelement.h"
#include "sequencer/randomelement.h"
#include "sequencer/soundelement.h"
#include "sequencer/oneshotelement.h"

struct SequencerPrivate;
class Sequencer : public QObject {
        Q_OBJECT
    public:
        explicit Sequencer(QObject* parent = nullptr);
        ~Sequencer();

        void addElement(SequencerElement* element);
        void addElement(QList<SequencerElement*> elements);

        void start();
        void render(QPainter* painter, QRect rect);

    signals:
        void requestRender();
        void done();

    private:
        SequencerPrivate* d;
};

#endif // SEQUENCER_H
