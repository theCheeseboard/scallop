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
#ifndef SOUNDELEMENT_H
#define SOUNDELEMENT_H

#include "sequencerelement.h"

struct SoundElementPrivate;
class SoundElement : public SequencerElement {
        Q_OBJECT
    public:
        explicit SoundElement(QString soundFile, QObject* parent = nullptr);
        ~SoundElement();

        static void setMute(bool mute);

    signals:

    private:
        SoundElementPrivate* d;

        // SequencerElement interface
    public:
        void run();
};

#endif // SOUNDELEMENT_H
