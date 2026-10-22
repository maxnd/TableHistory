<img align="top" src="https://github.com/maxnd/TableHistory/raw/main/icons/icon.png" width="128">

# TableHistory

Version 1.0.9, published on October 22 2026.

Author and copyright: Massimo Nardello, Modena (Italy) 2026.

TableHistory is a free and open-source app released under the GPLv3 license useful to manage items of historical events, documents and persons, and to show them in a diagram. Data can be easily retrieved and imported in the app from LLMs like Google Gemini.

The app has been written in C++ and Qt libraries with the support of Google Gemini. It runs on Windows and macOS, and the interface is currently only in English.

To use the app, download the latest package from [GitHub Releases](https://github.com/maxnd/TableHistory/releases/latest).

The package for Mac has been compiled for Mac with a Silicon chip (M1 or following), and is *not* notarized by Apple. To run it, see the [Apple instructions](https://support.apple.com/en-us/102445) (section “If you want to open an app that hasn’t been notarized or is from an unidentified developer”), or simply copy the package in the `Applications` folder and run in the terminal:

```
xattr -r -d com.apple.quarantine /Applications/TableHistory.app
```

---

See in the [wiki section](https://github.com/maxnd/TableHistory/wiki) a nice presentation of the app.

Listen to a [introductory podcast](https://github.com/maxnd/TableHistory/raw/main/podcast/Introducing_TableHistory.mp3) and [advanced podcast](https://github.com/maxnd/TableHistory/raw/main/podcast/Advanced_TableHistory.mp3) on the use of the app. Both the podcasts have been made with Google Gemini.

---

## Look of the app

On Mac in dark mode, the app looks like this:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot1.png)

The form to edit the items looks like this:

![](https://github.com/maxnd/TableHistory/blob/main/screenshots/screenshot2.png)


## Features

The app may create and manage many historical items in a grid at the left side of the interface. Each item has a kind field, whose value could be `Person`, `Document` or `Event`, a name, a beginning and end year, a place and a free-length formatted note. The `Event` kind may be used also for historical periods like Enlightenment. The range of the years is from -20000 (20.000 b.C.) to 3000 (3.000 c.e.). If the beginning year is equal to the end year, because the item refers to a single-year document or event, the end year will not be shown in the grid. The possibile author of a document may be typed before or after its name (title) among parentheses or after a separator.

The app is meant to manage very wide periods of time, so items’ temporal location is identified only by beginning and end years, not by months or days. If their location were more precise, the diagram would become unmanageable. So, detailed temporal information like months, days or hours is to be typed in the notes.

Anyway, if it's necessary to sort the items on a very detailed date, a workaround is to set normally the beginning and end year, type the specific date for sorting before the name of the items, and then sort them by name. For this trick to work properly, the date must be in the format `year - month - day`, followed possibly by the time in the format `hour - minutes`. The year must always be 4 digits long, the month and day always 2 digits long, and the hours and minutes always 2 digits long. For instance: `B.C. 0044/03/15 • Assassination of Julius Caesar`, or `C.E. 1953/06/02 • Coronation of Queen Elizabeth II`, or `C.E. 2001/09/11 08:46 • American Airlines Flight 11 crashes into the North Tower of the World Trade Center`.

The grid and the notes field below it are read-only to avoid unwanted changes to existing items. Each item may be created or edited in a form that is shown with the menu items `Items - Add...` or `Items - Edit...`, or with a double click on an existing item in the grid. In the notes, the internet links are properly formatted and functional, but not in the `Add item` / `Edit item` form used to create or edit an item to allow the user to change them more easily. In this form, it's possibile to increase or decrease the font size of the notes using the shortcuts `Ctrl` + `Alt` + `+` and `Ctrl+` + `Alt` + `-`, and paste the text from the clipboard as plain text with the proper popup menu item or the shortcut `Ctrl` + `Shift` + `V`. This last functionality allows to paste formatted text taken from another app without messing the formatting of the current note.

Click on a header of a column grid to sort the items in ascending and then in descending order on that column. To sort data on more columns, sort on each one of them starting from the last one. For instance, to have the items sorted by kind and start year, sort them on start year and then on kind.

More items may be selected by clicking on them while holding the `Shift` or `Command` buttons. To select all the items, use the shortcut `Ctrl / Meta` + `A`.Then they can be deleted, or copied in the clipboard to be pasted in another file. It's also possibile to search for an item containing a specified text in its name, place or notes, or select all the items that contain that text. This last function emulates a filter functionality, while maintaining the full view of all the items. See below for the convenient menu items and shortcuts.

In TableHistory, it's possible to delete individual items from a file or copy and paste some of them into another file without compromising the functionality of both files. For this reason, the app does not support links between items. In fact, if some items were deleted or copied to a new file, any existing links between them and other items might be broken.

This limitation is also a feature. It makes possible to quickly create sub-archives by copying and pasting only specific items from the current file into a new file, whether for focused analysis on a smaller amount of data or to share them with others. The absence of a linking functionality ensures that the file containing the subset of items remains fully functional, with no broken links.

To group records by argument or period, add tags to the notes, which can then be located using the search function of the app. To be sure that the search will work properly, insert the hash (or other symbol) also at the end of the word  (e.g. `#roman-history#`).

In the `Add item` / `Edit item` form, the buttons above the editor of the notes format the selected text as:

- heading 1 (shortcut: `Meta + 1` or `Ctrl + 1`);
- heading 2 (shortcut: `Meta + 2` or `Ctrl + 2`);
- heading 3 (shortcut: `Meta + 3` or `Ctrl + 3`);
- bold (shortcut: `Meta + B` or `Ctrl + B`);
- italics (shortcut: `Meta + I` or `Ctrl + I`);
- no formatting (shortcut: `Meta + 4` or `Ctrl + 4`).

To save data and close the form, use the button `OK` or the shortcut `Meta / Ctrl + Return`. Press `Esc` to discard changes.

At the right side of the main form of the app, the Gantt diagram shows the historical location of the various items. When some year is uncertain, the bar is printed not with a solid color, but as a light grid. The diagram may be zoomed (see the zoom bar at the bottom) and scrolled, and also exported in a vectorial PDF, always with a white background in spite of the theme in use in the operative system (light or dark). The PDF can be resized infinitely without loss of quality.

The selected items may be exported also as a PDF document.

Data are stored in a `.csv` file, so no database is used.

## Menu items

### File

- New: create a new file.
- Open...: open a existing file.
- Save: save the current file.
- Save as...: save the current file with a new name.
- Recent files: open the last 8 loaded files.
- Export selected items...: export the selected items as a PDF document.
- Export diagram...: export the diagram as a vectorial PDF.

### Items

- Add: add a new item.
- Edit: edit the current item (double click on it does the same).
- Delete: delete the selected items.
- Find...: find the first item that contains the specified text in the name, place or notes.
- Find next: find the next item that contains the text specified with the `Find` menu item in the name, place or notes.
- Find previous: find the previous item that contains the text specified with the `Find` menu item in the name, place or notes.
- Find and select...: select all the items that contain the specified text in the name, place or notes.
- Copy selected: copy in the clipboard all the selected items.
- Paste from selection: paste from the clipboard the items copied with the `Copy selected` menu item.

### View

- Zoom In Diagram: make the diagram wider.
- Zoom Out Diagram: make the diagram narrower.
- Reset Zoom Diagram: reset the zoom of the diagram.
- Zoom In Notes: Increase the size of the font of the notes below the grid.
- Zoom Out Notes: decrease the size of the font of the notes below the grid.
- Enter full screen: make the app full screen.

### Use of AI to gather data

It's possible to gather data from AI, like Gemini or Claude, copy them in the clipboard and paste them in the different fields of the app. To do so, use a prompt like this (tested on Gemini and Claude):

```
Provide 100 [or more, or less] cards of important persons, events and documents about [theme of interest] as a single code block whose first two lines must be:

TableHistoryItems:
Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes

Each card must be formatted strictly as a single-line CSV record matching this exact header structure:

Kind,Name,StartYear,StartUncertain,EndYear,EndUncertain,Place,Notes

Kind must be either Person (value: 2), Document (value: 1), or Event (value: 0).
StartYear and EndYear must be numbers.
StartYear must be equal to EndYear if it refers to a single-date event/document.
If a person is still alive, set his or her birth year both as StartYear and as EndYear.
For years b.C., use negative numbers.
StartUncertain and EndUncertain must be true or false.
Name and Place must be wrapped in double quotes if the contains spaces or commas.
Notes must contain a brief summary or a complete text formatted with basic HTML tags (like <b>, <i>, <p>) and wrapped in double quotes.

```
Then copy the result of the AI and paste it in the app with the menu item `Items - Paste from selection`.

## Troubleshooting

- Q. The zoom of the text of the notes doesn't work.
- A. Select all the text and remove its formatting with the last right button.

## Revision history

### Version 1.0.9

- The PDF documents now use Helvetica Neue or Arial fonts, have page numbers and a better look.
- In the PDF documents, when the end year of a person is equal to the beginning year, the first is not printed because the person is still alive.
- The background of the exported diagram is now white and not light gray.
- Bugs fixing: in the editor, the font size of the headings was different from the one in the main window.
- Minor improvements.

### Version 1.0.8

- Bugs fixing: the exportation of selected items in PDF didn't work properly (small fonts, just one page, etc.).
- Bugs fixing: in the editor, pasting from the clipboard removed the possible formatting of the text.
- Improved the style of the PDF exportation of selected items.
- Minor improvements.

### Version 1.0.7.1

- Bugs fixing.

### Version 1.0.7

- Added the `File - Export selected items to PDF...`.
- Added the zoom on the text of the notes.
- The diagram always has a white background.
- Improved documentation.
- Bugs fixing.

### Version 1.0.6

- Added `Recent files` menu item.
- Added `Find and select` menu item.
- Improved documentation.
- Bugs fixing.

[...]