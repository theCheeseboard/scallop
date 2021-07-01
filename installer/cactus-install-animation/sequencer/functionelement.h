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
#ifndef FUNCTIONELEMENT_H
#define FUNCTIONELEMENT_H

#include <functional>
#include "sequencerelement.h"

struct FunctionElementPrivate;
class FunctionElement : public SequencerElement {
        Q_OBJECT
    public:
        explicit FunctionElement(std::function<void()> function, QObject* parent = nullptr);
        ~FunctionElement();

    signals:

    private:
        FunctionElementPrivate* d;


        // SequencerElement interface
    public:
        void run();
};

#endif // FUNCTIONELEMENT_H
