const { app, BrowserWindow, ipcMain, desktopCapturer } = require('electron');
const { exec } = require('child_process');
const path = require('path');

function createWindow() {
  const win = new BrowserWindow({
    width: 1200,
    height: 700,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js')
    }
  });

  win.loadFile('index.html');
}

// Listen for scrcpy command from frontend
ipcMain.on('launch-scrcpy', (event, phoneIP) => {
  const cmd = `adb connect ${phoneIP}:5555 && scrcpy --tcpip=${phoneIP}:5555`;
  exec(cmd, (err, stdout, stderr) => {
    if (err) {
      console.error("Error running scrcpy:", err);
      return;
    }
    console.log(stdout);
  });
});

ipcMain.handle('get-screen-sources', async () => {
  return await desktopCapturer.getSources({ types: ['window', 'screen'] });
});

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});
