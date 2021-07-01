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
#include "textbox.h"

#include <QRect>
#include <QPainter>
#include <the-libs_global.h>
#include "zoomsvgrenderer.h"
#include "sequencer.h"

struct TextBoxPrivate {
    QStringList textParts;
    TextBox::TextBoxType type;
    ZoomSvgRenderer* boxRenderer;

    double maxWidth = 0;

    QString currentText;

    bool isRunning = false;
};

TextBox::TextBox(QStringList textParts, TextBoxType type, QObject* parent) : QObject(parent) {
    d = new TextBoxPrivate();
    d->textParts = textParts;
    d->type = type;

    switch (type) {
        case TextBox::NormalCharacter:
        case TextBox::RobotCharacter:
            d->boxRenderer = new ZoomSvgRenderer(":/installanim/standard-textbox.svg");
            break;
    }

    d->boxRenderer->setMode(Qt::KeepAspectRatio);
}

TextBox::~TextBox() {
    delete d;
}

void TextBox::trigger() {
    d->isRunning = true;

    Sequencer* sequencer = new Sequencer(this);

    sequencer->addElement(new AnimationElement(0, SC_DPI(800), 200, [ = ](QVariant value) {
        d->maxWidth = value.toDouble();
    }));

    for (const QString& text : qAsConst(d->textParts)) {
        sequencer->addElement(new FunctionElement([ = ] {
            d->currentText = "";
        }));
        for (QChar c : text) {
            sequencer->addElement({
                new FunctionElement([ = ] {
                    d->currentText += c;
                }),
                new PauseElement(50)
            });
        }
        sequencer->addElement(new PauseElement(3000));
    }

    sequencer->addElement({
        new AnimationElement(SC_DPI(800), 0, 200, [ = ](QVariant value) {
            d->maxWidth = value.toDouble();
        }),
        new FunctionElement([ = ] {
            d->isRunning = false;
        })
    });

    connect(sequencer, &Sequencer::requestRender, this, &TextBox::requestRender);
    connect(sequencer, &Sequencer::done, this, &TextBox::done);
    sequencer->start();
}

void TextBox::render(QPainter* painter, QRect rect) {
    if (!d->isRunning) return;

    QRect renderRect = rect;
    if (renderRect.width() > d->maxWidth) {
        renderRect.setWidth(d->maxWidth);
        renderRect.moveCenter(rect.center());
    }

    double ratio = static_cast<double>(d->boxRenderer->size().height()) / d->boxRenderer->size().width();
    renderRect.setHeight(renderRect.width() * ratio);

    d->boxRenderer->render(painter, renderRect);

    QRect textBounds = d->boxRenderer->bounds("textbounds", renderRect);

    QFont font = QApplication::font();

    switch (d->type) {
        case TextBox::NormalCharacter:
            painter->setPen(Qt::black);
            break;
        case TextBox::RobotCharacter:
            font.setFamily("JetBrains Mono");
            painter->setPen(Qt::green);
            break;
    }

    font.setPixelSize(textBounds.height() / 4);
    painter->setFont(font);

    painter->drawText(textBounds, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, d->currentText);
}
