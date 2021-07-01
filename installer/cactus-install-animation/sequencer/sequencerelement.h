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
#ifndef SEQUENCERELEMENT_H
#define SEQUENCERELEMENT_H

#include <QObject>

class QPainter;
class SequencerElement : public QObject {
        Q_OBJECT
    public:
        explicit SequencerElement(QObject* parent = nullptr);

        virtual void run() = 0;
        virtual void render(QPainter* painter, QRect rect);

    signals:
        void done();
        void requestRender();

};

#endif // SEQUENCERELEMENT_H
