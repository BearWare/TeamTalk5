package dk.bearware.backend;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothHeadset;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;

public class BluetoothHeadsetHelper {

    public interface HeadsetConnectionListener {
        public void onHeadsetConnected();
        public void onHeadsetDisconnected();
    }

    public interface ScoAudioConnectionListener {
        public void onScoAudioConnected();
        public void onScoAudioDisconnected();
    }


    private final Context context;
    private final AudioManager audioManager;
    private final BluetoothAdapter bluetoothAdapter;
    private final Set<HeadsetConnectionListener> headsetConnectionListeners;
    private final Set<ScoAudioConnectionListener> scoAudioConnectionListeners;

    private BluetoothHeadset bluetoothHeadset;

    private volatile boolean headsetConnected;
    private volatile boolean onHeadsetSco;

    private boolean scoAudioConnected;
    private boolean started;
    private boolean connectionEventsHandled;
    private boolean stateChangeEventsHandled;


    BluetoothHeadsetHelper(Context context) {
        this.context = context;
        audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        headsetConnectionListeners = new CopyOnWriteArraySet<HeadsetConnectionListener>();
        scoAudioConnectionListeners = new CopyOnWriteArraySet<ScoAudioConnectionListener>();
        headsetConnected = false;
        onHeadsetSco = false;
    }


    public boolean start() {
        if ((bluetoothAdapter != null) && bluetoothAdapter.isEnabled() && !started) {
            if (!bluetoothAdapter.getProfileProxy(context, headsetProfileListener, BluetoothProfile.HEADSET))
                bluetoothHeadset = null;
            started = true;
        }
        return started;
    }

    public void stop() {
        if (started) {
            scoAudioDisconnect();
            if ((bluetoothAdapter != null) && bluetoothAdapter.isEnabled() && (bluetoothHeadset != null))
                bluetoothAdapter.closeProfileProxy(BluetoothProfile.HEADSET, bluetoothHeadset);
            bluetoothHeadset = null;
            if (connectionEventsHandled) {
                context.unregisterReceiver(connectionEventReceiver);
                connectionEventsHandled = false;
            }
            if (headsetConnected)
                onHeadsetDisconnected();
            started = false;
        }
    }

    public synchronized boolean scoAudioConnect() {
        if (audioManager.isBluetoothScoAvailableOffCall() && headsetConnected && !scoAudioConnected) {
            context.registerReceiver(stateChangeEventReceiver, new IntentFilter(AudioManager.ACTION_SCO_AUDIO_STATE_UPDATED));
            audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
            audioManager.startBluetoothSco();
            stateChangeEventsHandled = true;
            scoAudioConnected = true;
        }
        return scoAudioConnected;
    }

    public synchronized void scoAudioDisconnect() {
        if (scoAudioConnected) {
            audioManager.stopBluetoothSco();
            audioManager.setMode(AudioManager.MODE_NORMAL);
            if (stateChangeEventsHandled) {
                context.unregisterReceiver(stateChangeEventReceiver);
                stateChangeEventsHandled = false;
            }
            scoAudioConnected = false;
        }
    }

    public boolean isStarted() {
        return started;
    }

    public boolean isHeadsetConnected() {
        return headsetConnected;
    }

    public boolean isOnHeadsetSco() {
        return onHeadsetSco;
    }

    public void registerHeadsetConnectionListener(HeadsetConnectionListener listener) {
        headsetConnectionListeners.add(listener);
    }

    public void unregisterHeadsetConnectionListener(HeadsetConnectionListener listener) {
        headsetConnectionListeners.remove(listener);
    }

    public void registerScoAudioConnectionListener(ScoAudioConnectionListener listener) {
        scoAudioConnectionListeners.add(listener);
    }

    public void unregisterScoAudioConnectionListener(ScoAudioConnectionListener listener) {
        scoAudioConnectionListeners.remove(listener);
    }


    private void onHeadsetConnected() {
        headsetConnected = true;
        for (HeadsetConnectionListener listener : headsetConnectionListeners)
            listener.onHeadsetConnected();
    }

    private void onHeadsetDisconnected() {
        headsetConnected = false;
        for (HeadsetConnectionListener listener : headsetConnectionListeners)
            listener.onHeadsetDisconnected();
    }

    private void onScoAudioConnected() {
        onHeadsetSco = true;
        for (ScoAudioConnectionListener listener : scoAudioConnectionListeners)
            listener.onScoAudioConnected();
    }

    private void onScoAudioDisconnected() {
        onHeadsetSco = false;
        for (ScoAudioConnectionListener listener : scoAudioConnectionListeners)
            listener.onScoAudioDisconnected();
    }


    private BroadcastReceiver connectionEventReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (BluetoothDevice.ACTION_ACL_CONNECTED.equals(action)) {
                BluetoothDevice connectedHeadset = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                BluetoothClass bluetoothClass = connectedHeadset.getBluetoothClass();
                if (bluetoothClass != null) {
                    int deviceClass = bluetoothClass.getDeviceClass();
                    if (deviceClass == BluetoothClass.Device.AUDIO_VIDEO_HANDSFREE || deviceClass == BluetoothClass.Device.AUDIO_VIDEO_WEARABLE_HEADSET) {
                        onHeadsetConnected();
                    }
                }
            } else if (BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action)) {
                onHeadsetDisconnected();
            } else if (BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED.equals(action)) {
                int state = intent.getIntExtra(BluetoothHeadset.EXTRA_STATE, BluetoothHeadset.STATE_DISCONNECTED);
                switch (state) {
                case BluetoothHeadset.STATE_CONNECTED:
                    onHeadsetConnected();
                    break;
                case BluetoothHeadset.STATE_DISCONNECTED:
                    onHeadsetDisconnected();
                    break;
                default:
                    break;
                }
            }
        }
    };

    private BroadcastReceiver stateChangeEventReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (AudioManager.ACTION_SCO_AUDIO_STATE_UPDATED.equals(action)) {
                int state = intent.getIntExtra(AudioManager.EXTRA_SCO_AUDIO_STATE, AudioManager.SCO_AUDIO_STATE_ERROR);
                switch (state) {
                case AudioManager.SCO_AUDIO_STATE_CONNECTED:
                    onScoAudioConnected();
                    break;
                case AudioManager.SCO_AUDIO_STATE_DISCONNECTED:
                    onScoAudioDisconnected();
                    break;
                default:
                    break;
                }
            }
        }
    };

    private BluetoothProfile.ServiceListener headsetProfileListener = new BluetoothProfile.ServiceListener() {
        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            if (profile == BluetoothProfile.HEADSET) {
                bluetoothHeadset = (BluetoothHeadset) proxy;
                if (!bluetoothHeadset.getConnectedDevices().isEmpty())
                    onHeadsetConnected();
                context.registerReceiver(connectionEventReceiver, new IntentFilter(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED));
                connectionEventsHandled = true;
            }
        }

        @Override
        public void onServiceDisconnected(int profile) {
            if (profile == BluetoothProfile.HEADSET) {
                bluetoothHeadset = null;
            }
        }
    };

}
