# Fife

Portable GUI framework and utilities.

## Project Layout

```
src/demo: A wee live demo of the framework. Contains main().
src/opt: Units that must be explicitly enabled, in local/config.mk.
src/lib: Unconditional units included in the library.
```

This project will build `libfife` and also `fifet`, our text editor.
I'm thinking the vast majority of the text editor's functionality will be implemented by `libfife`, so the actual app layer is thin enough to just ride along.

Once `fifet` works, I'll begin a new project `piccolo`, an image editor.

## TODO

- [ ] Generate public header.
- [ ] Test framework.
- [ ] Window Manager abstraction.
- - [ ] Copy/paste. Find an interface amenable to all particpants.
- - [ ] Can we get the double-click interval from window manager? Currently hard-coded to 500 ms in `gui_context.c`.
- [ ] Generic widget.
- - [ ] Events.
- - [x] Focus ring.
- - [ ] Focus ring should also be responsible for accelerators. And mind that where there's a label+field pair, we need to interact with both.
- [ ] Window Manager implementations.
- - [x] X11
- - - [x] Window is not getting keyboard focus by default. Was that just a fluke? ...seems so. Another run worked fine. We're using the same X11 logic as Egg v2.
- - [ ] Wayland
- - [ ] MacOS
- - [ ] MS Windows
- [ ] Text rendering.
- - [ ] Tofu.
- [ ] Widgets.
- - [x] Packer.
- - [x] Label.
- - [x] Button.
- - [x] Text field.
- - [x] Checkbox.
- - [x] Scroller. XXX All widgets have this capability.
- - [ ] Text editor.
- - [ ] Dialogue.
- - [ ] File dialogue.
- - [x] Menu bar.
- - [ ] Flexible panels. eg files list and editor canvas, with a draggable boundary.
- [ ] Basic utilities.
- - [x] Filesystem. As an `opt` unit, in case we port to something exotic.
- - [x] Serial.
- - [ ] Image.
- [ ] Plugin framework. WAMR?
- [ ] 15-pixel font. (15 significant pixels; probly more like 20 with descenders)
- [ ] Embed a few standard fonts in the library, but do preserve ability to load from a file.
- [ ] field: Different colors when blurred.
- [x] field: mouse

## Text Editor Requirements

- Integrated hex editor.
- Never refuse to open a file, except for like I/O and permission errors, out of memory, etc.
- In particular, do not reject files due to malformed encoding. "Yes! That's why I bloody need to edit it!"
- Detect encoding at open, allow user to override, and use a user-defined default for new files. UTF-8 out of the box.
- Files above a certain size, say 1 MB, can only open in the hex editor, and hex editor will allow 63-bit file sizes. (editing pagewise).
- Detect newline convention at open and use a user-defined default for new and empty files. LF out of the box.
- Optional autoindent: On Enter, emit a newline and then the indent of the previous line. HT and space.
- Tab key and deleting of space: My preference is Tab emits HT and backspace deletes one codepoint. Ask Alex what he prefers, probably want a few configurable options.
- Display of HT: I have no feelings on this, I never use HT or the Tab key.
- Multiple open files with a tab bar at the top, nothing surprising.
- Scroll bars visible always! Jesus fucking christ, why is this even a question. Pluma, and lots of modern software, only show the scroll bar when scrolling.
- Control-Arrow: Skip zero or more space, then one or more word until the next space. Lots of editors do it different and I will go to war over this.
- All UI must be keyboard-navigable. That's a general guideline at the GUI level anyway, but pay particular attention in the text editor. I don't want to ever need the mouse.
- Clear indication of which files are dirty, eg a star in their tab, and prompt when closing a dirty editor. As you'd expect.
- Store UI settings per-user, eg under `~/.config/fife/`. Pluma makes me set case-senstive for search every time it opens, a minor annoyance.
- Closing the last tab does not close the program. VSCode does that and it drives me crazy.
- Clicking in selected text drops the selection and is otherwise like any other click -- no dragging of text blocks, why on earth would anyone want that?
- Copy/paste using the platform's services. Plain text only. If we display highlighting, that never participates in copy/paste.
- - In hex editor, copy/paste operates on the hex text by default. Include an option, hold Shift or something, to copy/paste binary.

## Image Editor Requirements

- Open and save PNG files directly. GIMP 2.6 or so started requiring Import/Export for all non-GIMP file formats and it is bloody obnoxious.
- A selection is implicitly a floating layer. Dropping the selection anchors it.
- Zoom by integer multiples and factors only. Don't allow fractional zoom levels, where like some columns would be wider than others.
- Multiple levels of undo, and ancillary operations like selection participate too. GIMP sets an excellent example.
- Support as many well-defined formats as we can manage. PNG, GIF, BMP, and QOI are easy. Include libjpeg optionally at build and support JPEG.
- Raw data open and save too. Again, GIMP does this pretty well.
- Per-file selection of color space and palette. Be flexible about palettes. External file, built-in defaults, stored in image...
- Layer stack like GIMP, but always implicitly merge layers on save. No opinion, whether animated GIF frames should present as layers. We're not an animation tool.
- Preview window. eg show 1x view when zoomed in, user-defined animations, tiling preview.
- - ^ animation and tiling support would be a big win, I would use those every day.
- Option to view and edit alpha channel like a grayscale image. Like GIMP does with masks.
