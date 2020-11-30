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
#ifndef ONBOARDINGHOSTNAME_H
#define ONBOARDINGHOSTNAME_H

#include <onboardingpage.h>

namespace Ui {
    class OnboardingHostname;
}

class OnboardingHostname : public OnboardingPage {
        Q_OBJECT

    public:
        explicit OnboardingHostname(QWidget* parent = nullptr);
        ~OnboardingHostname();

    private slots:
        void on_titleLabel_backButtonClicked();

        void on_nextButton_clicked();

        void on_hostnameEdit_textChanged(const QString& arg1);

    private:
        Ui::OnboardingHostname* ui;

        QString generateStaticHostname(QString hostname);

        // OnboardingPage interface
    public:
        QString name();
        QString displayName();
};

#endif // ONBOARDINGHOSTNAME_H
