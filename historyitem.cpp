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

#include "historyitem.h"
#include <QStringList>
#include <QList>

QString HistoryItem::kindToString(ItemKind k) {
    switch (k) {
        case ItemKind::Document: return "Document";
        case ItemKind::Person:   return "Person";
        case ItemKind::Event:
        default:                 return "Event";
    }
}

ItemKind HistoryItem::stringToKind(const QString &str) {
    if (str.trimmed().compare("Document", Qt::CaseInsensitive) == 0) return ItemKind::Document;
    if (str.trimmed().compare("Person", Qt::CaseInsensitive) == 0)   return ItemKind::Person;
    return ItemKind::Event;
}

static QString escapeCsv(const QString &field) {
    QString escaped = field;
    if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r')) {
        escaped.replace('"', "\"\"");
        escaped = QString("\"%1\"").arg(escaped);
    }
    return escaped;
}

QString HistoryItem::toCsvRecord() const {
    // Helper lambda to escape CSV fields
    auto escapeCsv = [](const QString &field) {
        QString escaped = field;
        escaped.replace("\"", "\"\""); // Escape internal quotes
        // Enclose in quotes if it contains commas, quotes, or newlines
        if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r')) {
            escaped = "\"" + escaped + "\"";
        }
        return escaped;
    };

    QStringList fields;
    fields << QString::number(static_cast<int>(kind))
           << escapeCsv(name)
           << QString::number(startYear)
           << (startUncertain ? "true" : "false")
           << QString::number(endYear)
           << (endUncertain ? "true" : "false")
           << escapeCsv(place)
           << escapeCsv(notes); // Notes field properly escaped & quoted

    return fields.join(',');
}

HistoryItem HistoryItem::fromCsvRecord(const QStringList &fields) {
    HistoryItem item;
    if (fields.size() >= 8) {
        //item.kind = stringToKind(fields[0]);
        bool ok = false;
        int kindVal = fields.at(0).toInt(&ok);
        // Ensure it maps to a valid enum range, otherwise fallback or handle safely
        if (ok && kindVal >= 0 && kindVal <= 2) { // Adjust bounds based on your enum size
            item.kind = static_cast<ItemKind>(kindVal);
        } else {
            item.kind = ItemKind::Event; // or handle default
        }
        item.name = fields[1];
        item.startYear = fields[2].toInt();
        item.startUncertain = (fields[3].trimmed().compare("true", Qt::CaseInsensitive) == 0 || fields[3].trimmed() == "1");
        item.endYear = fields[4].toInt();
        item.endUncertain = (fields[5].trimmed().compare("true", Qt::CaseInsensitive) == 0 || fields[5].trimmed() == "1");
        item.place = fields[6];
        item.notes = fields[7];
    }
    return item;
}
