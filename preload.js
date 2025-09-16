const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  launchScrcpy: (phoneIP) => ipcRenderer.send('launch-scrcpy', phoneIP),
  getScreenSources: () => ipcRenderer.invoke('get-screen-sources')
});
