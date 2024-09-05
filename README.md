## Dependencies

- Qt6 >= 6.5
- Meson
- Ninja

## Building

Linux build:

1. run `make` or `make release` and binary is ready inside `build/linux/src` folder

Windows build:

1. Download qt source from [Qt website](https://download.qt.io/official_releases/qt/) and compile only qtbase and qttools.
2. Add Qt bin folder to `PATH` by editing `QT_BIN` variable in `build.ps1`
3. execute `.\build.ps1 release` and executable is produced inside `build/win/src` folder

## IPC

The IPC is using jsonrpc 2.0 and only communicate over WebSocket through port 31795.
Example usage can be found in the [test script](./test/websocket.js).
API documentation available in [wsserver.cpp](./src/ipc/wsserver.cpp) where
function name start with `method` or here in summary with aria2 documentation style:

### `open([secret,] uri)`

Open url or filepath using `xdg-open` in Unix and win32 cmd's `start` in Windows.

params:

```typescript
uri: string;
```

return: `"OK"`

### `delete([secret,] filepath)`

Delete file or folder.

```typescript
filepath: string;
```

return: `"OK"`

### `status([secret,] filepath)`

Get status of file.

params:

```typescript
filepath: string;
```

return:

```typescript
{
    type?: "folder" | "file",
    exist: boolean,
}
```

### `filePicker([secret,] type [, filter])`

Open a file picker pop-up and return selected file/folder, empty string if cancelled.

Since version 0.2.0

params:

```typescript
type: "file" | "folder",
filter: string, // https://doc.qt.io/qt-6/qfiledialog.html#getOpenFileName
```

return:

```typescript
{
    selected: string,
}
```

### `version([secret])`

Get aria2tray version.

return:

```typescript
{
    version: string,
}
```

### `ping()`

Ping the server.

return: `pong`
