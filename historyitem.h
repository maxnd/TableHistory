/*
* TableHistory 1.x
* Author and copyright (C): Massimo Nardello, Modena (Italy) 2026.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version. You can read the version 3
* of the Licence in http://www.gnu.org/licenses/gpl-3.0.txt
* or in the file Licence.txt included in the files of the
* source code of this software.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* This program has been created with the support of Google Gemini.
*
*/

#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QString>
#include <QStringList>

enum class ItemKind {
    Event,
    Document,
    Person
};

struct HistoryItem {
    ItemKind kind = ItemKind::Event;
    QString name;
    int startYear = 0;
    bool startUncertain = false;
    int endYear = 0;
    bool endUncertain = false;
    QString place;
    QString notes;

    static QString kindToString(ItemKind k);
    static ItemKind stringToKind(const QString &str);

    QString toCsvRecord() const;
    static HistoryItem fromCsvRecord(const QStringList &fields);
};

#endif // HISTORYITEM_H
