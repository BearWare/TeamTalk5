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
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Build;
import android.os.Environment;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class FileListAdapter
extends BaseAdapter
implements ClientListener, Comparator<RemoteFile> {

    private static final int REMOTE_FILE_VIEW_TYPE = 0,
        FILE_TRANSFER_VIEW_TYPE = 1,

        VIEW_TYPE_COUNT = 2;

    private final Context context;
    private final LayoutInflater inflater;
    private final NotificationManager notificationManager;
    private final AccessibilityAssistant accessibilityAssistant;

    private TeamTalkService ttService;
    private TeamTalkBase ttClient;
    private Vector<RemoteFile> remoteFiles;
    private Map<String, FileTransfer> downloads;
    private SparseArray<Notification.Builder> uploads;
    private volatile int chanId;
    private volatile boolean needRefresh;

    public FileListAdapter(Context uiContext, AccessibilityAssistant accessibilityAssistant) {
        context = uiContext;
        this.accessibilityAssistant = accessibilityAssistant;
        inflater = LayoutInflater.from(context);
        notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        remoteFiles = new Vector<RemoteFile>();
        downloads = new HashMap<String, FileTransfer>();
        uploads = new SparseArray<Notification.Builder>();
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
        uploads.clear();
        update();
    }

    public void update(Channel channel) {
        update((channel != null) ? channel.nChannelID : 0);
    }

    public void setTeamTalkService(TeamTalkService service) {
        remoteFiles.clear();
        downloads.clear();
        uploads.clear();
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
        for (int i = 0; i < uploads.size(); i++) {
            FileTransfer transfer = new FileTransfer();
            if (ttClient.getFileTransferInfo(uploads.keyAt(i), transfer))
                indicateUploadProgress(transfer);
        }
    }

    public String getRemoteName(String path) {
        File localFile = new File(path);
        String filename = localFile.getName();
        for (RemoteFile remoteFile : remoteFiles)
            if (remoteFile.szFileName.equals(filename))
                return filename;
        return null;
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
    public int getItemViewType(int position) {
        return isTransferring(remoteFiles.get(position)) ? FILE_TRANSFER_VIEW_TYPE : REMOTE_FILE_VIEW_TYPE;
    }

    @Override
    public int getViewTypeCount() {
        return VIEW_TYPE_COUNT;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final RemoteFile remoteFile = remoteFiles.get(position);
        View.OnClickListener buttonClickListener = new View.OnClickListener() {

                @Override
                public void onClick(View v) {
                    switch (v.getId()) {
                    case R.id.cancel_btn:
                        if (ttClient.cancelFileTransfer(downloads.get(remoteFile.szFileName).nTransferID)) {
                            downloads.remove(remoteFile.szFileName);
                            notifyDataSetChanged();
                        }
                        break;
                    case R.id.download_btn: {
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
                        break;
                    }
                    case R.id.remove_btn: {
                        AlertDialog.Builder alert = new AlertDialog.Builder(context);
                        alert.setMessage(context.getString(R.string.remote_file_remove_confirmation, remoteFile.szFileName));
                        alert.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {

                                @Override
                                public void onClick(DialogInterface dialog, int whichButton) {
                                    if (ttClient.doDeleteFile(chanId, remoteFile.nFileID) <= 0)
                                        Toast.makeText(context,
                                                       context.getString(R.string.err_file_delete,
                                                                         remoteFile.szFileName),
                                                       Toast.LENGTH_LONG).show();
                                }
                            });

                        alert.setNegativeButton(android.R.string.no, null);
                        alert.show();
                        break;
                    }
                    default:
                        break;
                    }
                }
            };

        switch (getItemViewType(position)) {
        case REMOTE_FILE_VIEW_TYPE: {
            if((convertView == null) || (convertView.findViewById(R.id.fileinfo) == null))
                convertView = inflater.inflate(R.layout.item_remote_file, null);
            ((TextView)convertView.findViewById(R.id.fileinfo)).setText(String.valueOf(remoteFile.nFileSize));
            Button downloadButton = (Button)convertView.findViewById(R.id.download_btn);
            Button removeButton = (Button)convertView.findViewById(R.id.remove_btn);
            downloadButton.setOnClickListener(buttonClickListener);
            downloadButton.setAccessibilityDelegate(accessibilityAssistant);
            removeButton.setOnClickListener(buttonClickListener);
            removeButton.setAccessibilityDelegate(accessibilityAssistant);
            break;
        }
        case FILE_TRANSFER_VIEW_TYPE: {
            if((convertView == null) || (convertView.findViewById(R.id.progress) == null))
                convertView = inflater.inflate(R.layout.item_file_transfer, null);
            FileTransfer transferinfo = downloads.get(remoteFile.szFileName);
            ((TextView)convertView.findViewById(R.id.progress)).setText(context.getString(R.string.download_progress, transferinfo.nTransferred * 100 / transferinfo.nFileSize));
            Button cancelButton = (Button)convertView.findViewById(R.id.cancel_btn);
            cancelButton.setOnClickListener(buttonClickListener);
            cancelButton.setAccessibilityDelegate(accessibilityAssistant);
            break;
        }
        default:
            break;
        }
        ((TextView)convertView.findViewById(R.id.filename)).setText(remoteFile.szFileName);
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

    @SuppressWarnings("fallthrough")
    @Override
    public void onFileTransfer(FileTransfer transfer) {
        if (transfer.bInbound) {
            if (transfer.nChannelID == chanId) {
                boolean progress = false;
                switch (transfer.nStatus) {
                case FileTransferStatus.FILETRANSFER_ERROR:
                    warnDownloadFailure(transfer.szRemoteFileName);
                case FileTransferStatus.FILETRANSFER_CLOSED:
                    if (downloads.containsKey(transfer.szRemoteFileName)) {
                        downloads.remove(transfer.szRemoteFileName);
                        File localFile = new File(transfer.szLocalFilePath);
                        if (localFile.exists())
                            localFile.delete();
                    }
                    break;
                case FileTransferStatus.FILETRANSFER_ACTIVE:
                    progress = downloads.containsKey(transfer.szRemoteFileName);
                    downloads.put(transfer.szRemoteFileName, transfer);
                    break;
                case FileTransferStatus.FILETRANSFER_FINISHED:
                    downloads.remove(transfer.szRemoteFileName);
                    Toast.makeText(context,
                                   context.getString(R.string.download_succeeded,
                                                     transfer.szRemoteFileName,
                                                     transfer.szLocalFilePath),
                                   Toast.LENGTH_LONG).show();
                default:
                    break;
                }
                if (progress && accessibilityAssistant.isUiUpdateDiscouraged()) {
                    needRefresh = true;
                }
                else {
                    notifyDataSetChanged();
                }
            }
        }
        else {
            Notification.Builder progressNotification = uploads.get(transfer.nTransferID);
            switch (transfer.nStatus) {
            case FileTransferStatus.FILETRANSFER_ERROR:
                Toast.makeText(context,
                               context.getString(R.string.upload_failed,
                                                 transfer.szLocalFilePath),
                               Toast.LENGTH_LONG).show();
            case FileTransferStatus.FILETRANSFER_CLOSED:
                if (progressNotification != null) {
                    notificationManager.cancel(transfer.nTransferID);
                    uploads.remove(transfer.nTransferID);
                }
                break;
            case FileTransferStatus.FILETRANSFER_ACTIVE:
                if (progressNotification == null) {
                    progressNotification = new Notification.Builder(context);
                    Intent cancellationIntent = new Intent(context, TeamTalkService.class);
                    int id = transfer.nTransferID;
                    cancellationIntent.putExtra(TeamTalkService.CANCEL_TRANSFER, id);
                    progressNotification.setSmallIcon(android.R.drawable.stat_sys_upload)
                        .setContentTitle(context.getString(R.string.upload_progress_title, transfer.szRemoteFileName))
                        .setContentIntent(PendingIntent.getService(context, 0, cancellationIntent, 0));
                    if (Build.VERSION.SDK_INT >= 17)
                        progressNotification.setShowWhen(false);
                    uploads.put(id, progressNotification);
                }
                indicateUploadProgress(transfer);
                break;
            case FileTransferStatus.FILETRANSFER_FINISHED:
                progressNotification.setSmallIcon(android.R.drawable.stat_sys_upload_done)
                    .setContentText(context.getString(R.string.complete))
                    .setProgress(0, 0, false)
                    .setContentIntent(PendingIntent.getActivity(context, 0, new Intent(), 0))
                    .setAutoCancel(true);
                notificationManager.notify(transfer.nTransferID, progressNotification.build());
                uploads.remove(transfer.nTransferID);
                Toast.makeText(context,
                               context.getString(R.string.upload_succeeded,
                                                 transfer.szLocalFilePath),
                               Toast.LENGTH_LONG).show();
            default:
                break;
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

    private void indicateUploadProgress(FileTransfer transfer) {
        int id = transfer.nTransferID;
        int percentage = (int)(transfer.nTransferred * 100 / transfer.nFileSize);
        notificationManager.notify(id, uploads.get(id).setContentText(context.getString(R.string.upload_progress, percentage)).setProgress(100, percentage, false).build());
    }

}
