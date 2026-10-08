<img align="top" src="https://github.com/maxnd/TableHistory/raw/main/resources/icon.png" width="128">

# TableHistory

Version 1.0.0, published on October 8 2026.

Author and copyright: Massimo Nardello, Modena (Italy) 2026.

TableHistory is a free and open-source app released under the GPLv3 license useful to manage items of historical events, documents and persons, and to show them in a Gantt diagram. The app has been written in C++ and Qt libraries with the support of Google Gemini, it has been compiled only for macOS and the interface is in English.

Download the latest version of the app for Mac from [GitHub Releases](https://github.com/maxnd/TableHistory/releases/latest) and copy it in the `Applications` folder.

The available package of the app has been compiled for Mac with Silicon chip (M1 or following), and is *not* notarized by Apple. To run it, see the [Apple instructions](https://support.apple.com/en-us/102445) (section “If you want to open an app that hasn’t been notarized or is from an unidentified developer”), or simply copy the package in the `Applications` folder and run in the terminal:

```
xattr -r -d com.apple.quarantine /Applications/TableHistory.app
```

## Look of the app

The app looks like this, in dark mode:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot1.png)

The form to edit the items looks like this:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot2.png)


## Features

The app may manage many items of persons, documents and events in a grid at the left side of the interface. Each item may have a name, a beginning and end year - the last one may be omitted -, a place and some formatted notes.

The grid is read-only, and each item my be edited within a form that is shown with a double click on it. Click on a header of a column of the grid to sort the items in ascending and then in descending order on that column.

More items may be selected by clicking on them while holding the `Shift` or `Command` buttons. Then they be deleted or copied in the clipboard to be pasted in another file. It's also possibile to search for an item containing a text in its name, place or notes.

The buttons above the editor of the notes allow formatting of the text as heading 1, 2 or 3, bold, italics and to remove formatting.

In the right side of the app, the Gantt diagram shows the historical location of the various items. The diagram may be zoomed (see the zoom bar at the bottom) and scrolled.

 The data are stored in a `.csv` file, so no database is used.

## Menu items

### File

- New: create a new file.
- Open: open a existing file.
- Save: save the current file.
- Save as: save the current file with a new name.
- Export diagram: export as a picture in `.bmp` format the diagram of the current file.

### Edit

- Cut: cut the selected text in the `Item notes` field.
- Copy: copy the selected text in the `Item notes` field.
- Paste: paste the text in the clipboard in the `Item notes` field.
- Select all: select all the text in the `Item notes` field.

### Items

- Add item: add a new item.
- Edit item: edit the current item (the double click on it does the same).
- Delete items: delete the selected items.
- Copy selected: copy in the clipboard the selected items.
- Paste from selection: paste from the clipboard the items copied with the previous functionality.
- Find: find the first item that contains the specified text in the name, place or notes.
- Find next: find the next item that contains the specified text in the name, place or notes.

### View
- Zoom In diagram: make the diagram wider.
- Zoom Out Diagram: make the diagram more narrow.
- Reset Zoom Diagram: reset the zoom of the diagram.
- Enter full screen: make the app fuul screen.

### Use of AI to gather data

It's possible to gather data from AI (like Gemini), copy them in the clipboard and paste them in the different fields of the app. To do so, use a prompt like this:

```
Provide information about [person, event or document] formatted strictly as a single line CSV record matching this exact header structure:

TableHistoryItems:
Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes

Kind must be either Person (value: 2), Document (value: 1), or Event (value: 0).
StartYear and EndYear must be numbers (skip the second if it's a single-date event/document).
StartUncertain and EndUncertain must be true or false.
Place must be wrapped in double quotes if it contains spaces or commas.
Notes must contain a brief summary or a complete text formatted with basic HTML tags (like <b>, <i>, <p>) and wrapped in double quotes.
```

Then copy the result of the AI and paste it in the app with the menu item `Items - Paste from selection`.
