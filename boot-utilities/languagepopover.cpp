/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
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
#include "languagepopover.h"
#include "ui_languagepopover.h"

struct LanguagePopoverPrivate {
    QLocale::Language currentLanguage;
};

LanguagePopover::LanguagePopover(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LanguagePopover) {
    ui->setupUi(this);

    d = new LanguagePopoverPrivate();
    ui->titleLabel->setBackButtonShown(true);
    ui->languageTitle->setBackButtonShown(true);
    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    QList<QListWidgetItem*> languages;
    for (int i = QLocale::C + 1; i < QLocale::LastLanguage; i++) {
#if !QT_VERSION_CHECK(5, 15, 0)
        if (i == QLocale::UncodedLanguages) continue;
#endif
        QLocale locale(static_cast<QLocale::Language>(i));
        if (locale.language() != i) continue;
        if (!locale.nativeLanguageName().isEmpty()) {
            QString text;
            QString native = locale.nativeLanguageName();
            if (locale.language() == QLocale::English) {
                text = "English"; //Do not localise
            } else if (native.isEmpty()) {
                text = QLocale::languageToString(locale.language());
            } else {
                text = QStringLiteral("%1 (%2)").arg(native).arg(QLocale::languageToString(locale.language()));
            }

            QListWidgetItem* item = new QListWidgetItem();
            item->setText(text);
            item->setData(Qt::UserRole, i);
            languages.append(item);
        }
    }

    std::sort(languages.begin(), languages.end(), [ = ](const QListWidgetItem * first, const QListWidgetItem * second) {
        return first->text().localeAwareCompare(second->text()) < 0;
    });

    for (QListWidgetItem* item : languages) {
        ui->languageSelection->addItem(item);
    }
}

LanguagePopover::~LanguagePopover() {
    delete d;
    delete ui;
}

void LanguagePopover::on_titleLabel_backButtonClicked() {
    emit rejected();
}

void LanguagePopover::on_languageSelection_itemActivated(QListWidgetItem* item) {
    QLocale::Language lang = static_cast<QLocale::Language>(item->data(Qt::UserRole).toInt());
    d->currentLanguage = lang;
    QList<QLocale::Country> countries = QLocale::countriesForLanguage(lang);
    if (countries.count() == 1) {
        emit accepted(QLocale(lang, countries.first()));
    } else {
        ui->localeSelection->clear();

        for (QLocale::Country country : countries) {
            QLocale locale(lang, country);
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(QStringLiteral("%1 (%2)").arg(QLocale::countryToString(country)).arg(locale.nativeCountryName()));
            item->setData(Qt::UserRole, country);
            ui->localeSelection->addItem(item);
        }

        ui->languageTitle->setText(QLocale::languageToString(lang));
        ui->stackedWidget->setCurrentWidget(ui->localePage);
    }
}

void LanguagePopover::on_localeSelection_itemActivated(QListWidgetItem* item) {
    QLocale::Country country = static_cast<QLocale::Country>(item->data(Qt::UserRole).toInt());
    emit accepted(QLocale(d->currentLanguage, country));
}

void LanguagePopover::on_languageTitle_backButtonClicked() {
    ui->stackedWidget->setCurrentWidget(ui->languagePage);
}
