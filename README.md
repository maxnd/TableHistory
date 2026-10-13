<img align="top" src="https://github.com/maxnd/TableHistory/raw/main/icons/icon.png" width="128">

# TableHistory

Version 1.0.4, published on October 13 2026.

Author and copyright: Massimo Nardello, Modena (Italy) 2026.

TableHistory is a free and open-source app released under the GPLv3 license useful to manage items of historical events, documents and persons, and to show them in a diagram. The app has been written in C++ and Qt libraries with the support of Google Gemini. It runs on Windows and macOS, and the interface is currently only in English.

To use the app, download the latest package from [GitHub Releases](https://github.com/maxnd/TableHistory/releases/latest).

The package for Mac has been compiled for Mac with a Silicon chip (M1 or following), and is *not* notarized by Apple. To run it, see the [Apple instructions](https://support.apple.com/en-us/102445) (section “If you want to open an app that hasn’t been notarized or is from an unidentified developer”), or simply copy the package in the `Applications` folder and run in the terminal:

```
xattr -r -d com.apple.quarantine /Applications/TableHistory.app
```

## Look of the app

On Mac in dark mode, the app looks like this:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot1.png)

The form to edit the items looks like this:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot2.png)


## Features

The app may create and manage many items of persons, documents and events in a grid at the left side of the interface. Each item may have a name, a beginning and end year, a place and some formatted notes. The range of the years is from -20000 (20.000 b.C.) to 3000 (3.000 c.e.). If the beginning year is equal to the end year, because the item refers to a single-year document or event, the end year will not be shown in the grid.

The beginning and end year are meant just to put the person, document or event meaningfully in the diagram. More detailed temporal information, like months or days, is to be typed in the notes.

The grid and the notes field below it are read-only. Each item may be created or edited in a form that is shown with the menu items `Items - Add...` or `Items - Edit...`, or with a double click on an existing item in the grid. In the notes, the internet links are properly formatted and functional, but not in the `Add item` / `Edit item` form used to create or edit an item.

Click on a header of a column grid to sort the items in ascending and then in descending order on that column. To sort data on more columns, sort on each one of them starting from the last one. For instance, to have the items sorted by kind and start year, sort them on start year and then on kind.

More items may be selected by clicking on them while holding the `Shift` or `Command` buttons. Then they can be deleted, or copied in the clipboard to be pasted in another file. It's also possibile to search for an item containing a text in its name, place or notes. See below for the convenient menu items and shortcuts.

In the `Add item` / `Edit item` form, the buttons above the editor of the notes format the selected text as:

- heading 1 (shortcut: `Meta + 1` or `Ctrl + 1`);
- heading 2 (shortcut: `Meta + 2` or `Ctrl + 2`);
- heading 3 (shortcut: `Meta + 3` or `Ctrl + 3`);
- bold (shortcut: `Meta + B` or `Ctrl + B`);
- italics (shortcut: `Meta + I` or `Ctrl + I`);
- no formatting (shortcut: `Meta + 4` or `Ctrl + 4`).

To save data and close the form, use the button `OK` or the shortcut `Meta / Ctrl + Return`. Press `Esc` to discard changes.

At the right side of the main form of the app, the Gantt diagram shows the historical location of the various items. The diagram may be zoomed (see the zoom bar at the bottom) and scrolled and exported in a vectorial PDF.

Data are stored in a `.csv` file, so no database is used.

## Menu items

### File

- New: create a new file.
- Open: open a existing file.
- Save: save the current file.
- Save as: save the current file with a new name.
- Export diagram: export the diagram as a vectorial PDF.

### Items

- Add: add a new item.
- Edit: edit the current item (double click on it does the same).
- Delete: delete the selected items.
- Copy selected: copy in the clipboard the selected items.
- Paste from selection: paste from the clipboard the items copied with the previous functionality.
- Find: find the first item that contains the specified text in the name, place or notes.
- Find next: find the next item that contains the specified text in the name, place or notes.

### View

- Zoom In Diagram: make the diagram wider.
- Zoom Out Diagram: make the diagram narrower.
- Reset Zoom Diagram: reset the zoom of the diagram.
- Enter full screen: make the app full screen.

### Use of AI to gather data

It's possible to gather data from AI, like Gemini, copy them in the clipboard and paste them in the different fields of the app. To do so, use a prompt like this (tested on Gemini):

```

Provide 100 [or more, or less] cards of important people, events and documents about [theme of interest]. Each card must be formatted strictly as a single line CSV record matching this exact header structure and adding TableHistoryItem: at the beginning:

TableHistoryItems:
Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes

Kind must be either Person (value: 2), Document (value: 1), or Event (value: 0).
StartYear and EndYear must be numbers. Set the second as the first if it's a single-date event/document. For years b.C., use negative numbers.
StartUncertain and EndUncertain must be true or false.
Place must be wrapped in double quotes if it contains spaces or commas.
Notes must contain a brief summary or a complete text formatted with basic HTML tags (like <b>, <i>, <p>) and wrapped in double quotes.
```

Then copy the result of the AI and paste it in the app with the menu item `Items - Paste from selection`.
