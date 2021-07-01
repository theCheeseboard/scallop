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
#include "soundelement.h"

#include <QSoundEffect>

struct SoundElementPrivate {
    QSoundEffect* effect;
};

SoundElement::SoundElement(QString soundFile, QObject* parent) : SequencerElement(parent) {
    d = new SoundElementPrivate();
    d->effect = new QSoundEffect(this);
    d->effect->setSource(QUrl(soundFile));
    connect(d->effect, &QSoundEffect::playingChanged, this, [ = ] {
        if (!d->effect->isPlaying()) emit done();
    });
}

SoundElement::~SoundElement() {
    delete d;
}


void SoundElement::run() {
    d->effect->play();
}
