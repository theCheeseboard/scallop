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
#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <QObject>

class QPainter;
struct TextBoxPrivate;
class TextBox : public QObject {
        Q_OBJECT
    public:
        enum TextBoxType {
            NormalCharacter,
            RobotCharacter
        };

        explicit TextBox(QStringList textParts, TextBoxType type = NormalCharacter, QObject* parent = nullptr);
        ~TextBox();

        void trigger();

        void render(QPainter* painter, QRect rect);

    signals:
        void requestRender();
        void done();

    private:
        TextBoxPrivate* d;
};

#endif // TEXTBOX_H
