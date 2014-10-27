package dk.bearware.data;

import java.io.File;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import dk.bearware.Channel;
import dk.bearware.ClientErrorMsg;
import dk.bearware.FileTransfer;
import dk.bearware.FileTransferStatus;
import dk.bearware.MediaFileInfo;
import dk.bearware.RemoteFile;
import dk.bearware.TeamTalkBase;
import dk.bearware.backend.TeamTalkService;
import dk.bearware.events.ClientListener;
import dk.bearware.gui.AccessibilityAssistant;
import dk.bearware.gui.R;
import dk.bearware.gui.Utils;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Environment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class FileListAdapter
extends BaseAdapter
implements ClientListener, Comparator<RemoteFile> {

    private final Context context;
    private final LayoutInflater inflater;
    private final AccessibilityAssistant accessibilityAssistant;

    private TeamTalkService ttService;
    private TeamTalkBase ttClient;
    private Vector<RemoteFile> remoteFiles;
    private Map<String, FileTransfer> downloads;
    private volatile int chanId;
    private volatile boolean needRefresh;

    public FileListAdapter(Context uiContext, AccessibilityAssistant accessibilityAssistant) {
        context = uiContext;
        this.accessibilityAssistant = accessibilityAssistant;
        inflater = LayoutInflater.from(context);
        remoteFiles = new Vector<RemoteFile>();
        downloads = new HashMap<String, FileTransfer>();
        chanId = 0;
        needRefresh = false;
    }

    public void update() {
        if (ttService != null) {
            remoteFiles = Utils.getRemoteFiles(chanId, ttService.getRemoteFiles());
            Collections.sort(remoteFiles, this);
        }
        notifyDataSetChanged();
    }

    public void update(int chanId) {
        this.chanId = chanId;
        downloads.clear();
        update();
    }

    public void update(Channel channel) {
        update((channel != null) ? channel.nChannelID : 0);
    }

    public void setTeamTalkService(TeamTalkService service) {
        remoteFiles.clear();
        downloads.clear();
        if (ttService != null)
            ttService.unregisterClientListener(this);
        if (service != null) {
            ttClient = service.getTTInstance();
            service.registerClientListener(this);
        }
        ttService = service;
    }

    public void performPendingUpdate() {
        for (FileTransfer transfer : downloads.values())
            if (ttClient.getFileTransferInfo(transfer.nTransferID, transfer))
                needRefresh = true;
        if (needRefresh) {
            accessibilityAssistant.lockEvents();
            notifyDataSetChanged();
            accessibilityAssistant.unlockEvents();
        }
    }


    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
        needRefresh = false;
    }

    @Override
    public int getCount() {
        return remoteFiles.size();
    }

    @Override
    public Object getItem(int position) {
        return remoteFiles.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final RemoteFile remoteFile = remoteFiles.get(position);

        if(convertView == null)
            convertView = inflater.inflate(R.layout.item_remote_file, null);

        TextView filename = (TextView) convertView.findViewById(R.id.filename);
        TextView fileinfo = (TextView) convertView.findViewById(R.id.fileinfo);
        Button actionButton = (Button) convertView.findViewById(R.id.action_btn);

        filename.setText(remoteFile.szFileName);
        if (isTransferring(remoteFile)) {
            FileTransfer transferinfo = downloads.get(remoteFile.szFileName);
            fileinfo.setText(context.getString(R.string.download_progress, transferinfo.nTransferred * 100 / transferinfo.nFileSize));
            actionButton.setText(android.R.string.cancel);
        }
        else {
            fileinfo.setText(String.valueOf(remoteFile.nFileSize));
            actionButton.setText(R.string.button_download);
        }

        actionButton.setOnClickListener(new OnClickListener() {

                @Override
                public void onClick(View v) {
                    if (isTransferring(remoteFile)) {
                        if (ttClient.cancelFileTransfer(downloads.get(remoteFile.szFileName).nTransferID)) {
                            downloads.remove(remoteFile.szFileName);
                            notifyDataSetChanged();
                        }
                    }

                    else {
                        File dlPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
                        if (dlPath.mkdirs() || dlPath.isDirectory()) {
                            final File localFile = new File(dlPath, remoteFile.szFileName);
                            if (localFile.exists()) {
                                AlertDialog.Builder alert = new AlertDialog.Builder(context);
                                alert.setMessage(context.getString(R.string.alert_file_override, localFile.getAbsolutePath()));
                                alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                                        @Override
                                        public void onClick(DialogInterface dialog, int whichButton) {
                                            if (localFile.delete()) {
                                                startDownload(remoteFile, localFile);
                                            }
                                            else {
                                                Toast.makeText(context,
                                                               context.getString(R.string.err_file_delete,
                                                                                 localFile.getAbsolutePath()),
                                                               Toast.LENGTH_LONG).show();
                                            }
                                        }
                                    });

                                alert.setNegativeButton(android.R.string.no, null);
                                alert.show();
                            }

                            else {
                                startDownload(remoteFile, localFile);
                            }
                        }

                        else {
                            Toast.makeText(context,
                                           context.getString(R.string.err_download_path,
                                                             dlPath.getAbsolutePath()),
                                           Toast.LENGTH_LONG).show();
                        }
                    }
                }
            });

        actionButton.setAccessibilityDelegate(accessibilityAssistant);
        convertView.setAccessibilityDelegate(accessibilityAssistant);
        return convertView;
    }


    @Override
    public void onInternalError(ClientErrorMsg clienterrormsg) {
    }

    @Override
    public void onVoiceActivation(boolean bVoiceActive) {
    }

    @Override
    public void onHotKeyToggle(int nHotKeyID, boolean bActive) {
    }

    @Override
    public void onHotKeyTest(int nVkCode, boolean bActive) {
    }

    @Override
    public void onFileTransfer(FileTransfer filetransfer) {
        if (filetransfer.bInbound && filetransfer.nChannelID == chanId) {
            boolean progress = false;
            switch (filetransfer.nStatus) {
            case FileTransferStatus.FILETRANSFER_ERROR:
                warnDownloadFailure(filetransfer.szRemoteFileName);
            case FileTransferStatus.FILETRANSFER_CLOSED:
                if (downloads.containsKey(filetransfer.szRemoteFileName)) {
                    downloads.remove(filetransfer.szRemoteFileName);
                    File localFile = new File(filetransfer.szLocalFilePath);
                    if (localFile.exists())
                        localFile.delete();
                }
                break;
            case FileTransferStatus.FILETRANSFER_ACTIVE:
                progress = downloads.containsKey(filetransfer.szRemoteFileName);
                downloads.put(filetransfer.szRemoteFileName, filetransfer);
                break;
            case FileTransferStatus.FILETRANSFER_FINISHED:
                downloads.remove(filetransfer.szRemoteFileName);
                Toast.makeText(context,
                               context.getString(R.string.download_succeeded,
                                                   filetransfer.szRemoteFileName,
                                                   filetransfer.szLocalFilePath),
                               Toast.LENGTH_LONG).show();
                break;
            default:
                break;
            }
            if (progress && accessibilityAssistant.uiUpdatesDiscouraged()) {
                needRefresh = true;
            }
            else {
                notifyDataSetChanged();
            }
        }
    }

    @Override
    public void onDesktopWindowTransfer(int nSessionID, int nTransferRemaining) {
    }

    @Override
    public void onStreamMediaFile(MediaFileInfo mediafileinfo) {
    }


    @Override
    public int compare(RemoteFile f1, RemoteFile f2) {
        return f1.szFileName.compareToIgnoreCase(f2.szFileName);
    }


    private boolean isTransferring(RemoteFile file) {
        return downloads.containsKey(file.szFileName);
    }

    private void warnDownloadFailure(String filename) {
        Toast.makeText(context,
                       context.getString(R.string.download_failed,
                                         filename),
                       Toast.LENGTH_LONG).show();
    }

    private void startDownload(RemoteFile remoteFile, File localFile) {
        if (ttClient.doRecvFile(chanId, remoteFile.nFileID, localFile.getAbsolutePath()) <= 0)
            warnDownloadFailure(remoteFile.szFileName);
    }

}
