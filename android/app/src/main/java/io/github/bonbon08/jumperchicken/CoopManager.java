package io.github.bonbon08.jumperchicken;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.ArrayAdapter;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

/**
 * Local two-player co-op over Bluetooth Classic (RFCOMM/SPP): one phone hosts, the other joins,
 * and both exchange their chicken's position/direction/score a few times a second once connected.
 * There is no synced level: each device still runs its own run, this just draws the other
 * player's chicken as a "ghost" so both screens feel like one shared space.
 */
class CoopManager {
    private static final String TAG = "JumperChickenCoop";
    private static final UUID APP_UUID = UUID.fromString("7e5c1b1e-2f0a-4c3a-9a2b-5e1f0a2b3c4d");
    private static final String SERVICE_NAME = "JumperChickenCoop";
    static final int RC_DISCOVERABLE = 9100;

    private final Activity activity;
    private final Handler mainHandler = new Handler(Looper.getMainLooper());

    private BluetoothAdapter adapter;
    private AcceptThread acceptThread;
    private ConnectThread connectThread;
    private volatile ConnectedThread connectedThread;
    private BroadcastReceiver discoveryReceiver;
    private AlertDialog joinDialog;

    private volatile boolean connected = false;
    private volatile float peerX = 0, peerY = 0;
    private volatile int peerDirection = 0, peerScore = 0;

    CoopManager(Activity activity) {
        this.activity = activity;
        BluetoothManager manager = (BluetoothManager) activity.getSystemService(Context.BLUETOOTH_SERVICE);
        adapter = manager != null ? manager.getAdapter() : null;
    }

    // ---- Called from native code (game thread) ----

    boolean hasBluetooth() {
        return adapter != null;
    }

    boolean hasPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            return ContextCompat.checkSelfPermission(activity, android.Manifest.permission.BLUETOOTH_CONNECT)
                    == PackageManager.PERMISSION_GRANTED
                    && ContextCompat.checkSelfPermission(activity, android.Manifest.permission.BLUETOOTH_SCAN)
                    == PackageManager.PERMISSION_GRANTED;
        }
        return ContextCompat.checkSelfPermission(activity, android.Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED;
    }

    void requestPermissions() {
        mainHandler.post(() -> {
            String[] perms = Build.VERSION.SDK_INT >= Build.VERSION_CODES.S
                    ? new String[]{ android.Manifest.permission.BLUETOOTH_CONNECT, android.Manifest.permission.BLUETOOTH_SCAN }
                    : new String[]{ android.Manifest.permission.ACCESS_FINE_LOCATION };
            ActivityCompat.requestPermissions(activity, perms, 9101);
        });
    }

    /** Makes this device discoverable (best effort) and waits for an incoming connection. */
    void startHost() {
        if (!hasBluetooth() || !hasPermissions()) return;
        disconnect();
        mainHandler.post(() -> {
            try {
                Intent discoverable = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
                discoverable.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 120);
                activity.startActivityForResult(discoverable, RC_DISCOVERABLE);
            } catch (SecurityException e) {
                Log.w(TAG, "Could not request discoverable mode: " + e.getMessage());
            }
            acceptThread = new AcceptThread();
            acceptThread.start();
        });
    }

    /** Shows a picker (paired + nearby devices) and connects to whichever the player taps. */
    void startJoin() {
        if (!hasBluetooth() || !hasPermissions()) return;
        disconnect();
        mainHandler.post(this::showDeviceList);
    }

    boolean isConnected() {
        return connected;
    }

    void send(float x, float y, int direction, int score) {
        ConnectedThread thread = connectedThread;
        if (thread != null) thread.write(x, y, direction, score);
    }

    float getPeerX() { return peerX; }
    float getPeerY() { return peerY; }
    int getPeerDirection() { return peerDirection; }
    int getPeerScore() { return peerScore; }

    void disconnect() {
        connected = false;
        if (acceptThread != null) { acceptThread.cancel(); acceptThread = null; }
        if (connectThread != null) { connectThread.cancel(); connectThread = null; }
        if (connectedThread != null) { connectedThread.cancel(); connectedThread = null; }
        stopDiscovery();
    }

    // ---- Device picker ----

    @SuppressWarnings("MissingPermission")   // guarded by hasPermissions() in startJoin()
    private void showDeviceList() {
        List<BluetoothDevice> devices = new ArrayList<>();
        ArrayAdapter<String> listAdapter = new ArrayAdapter<>(activity, android.R.layout.simple_list_item_1);

        try {
            Set<BluetoothDevice> bonded = adapter.getBondedDevices();
            if (bonded != null) {
                for (BluetoothDevice device : bonded) {
                    devices.add(device);
                    listAdapter.add(device.getName() + " (paired)");
                }
            }
        } catch (SecurityException e) {
            Log.w(TAG, "Could not read paired devices: " + e.getMessage());
        }

        discoveryReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (!BluetoothDevice.ACTION_FOUND.equals(intent.getAction())) return;
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (device == null || devices.contains(device)) return;
                devices.add(device);
                String name;
                try {
                    name = device.getName();
                } catch (SecurityException e) {
                    name = null;
                }
                listAdapter.add((name != null ? name : device.getAddress()) + " (nearby)");
            }
        };
        activity.registerReceiver(discoveryReceiver, new IntentFilter(BluetoothDevice.ACTION_FOUND));
        try {
            adapter.startDiscovery();
        } catch (SecurityException e) {
            Log.w(TAG, "Could not start discovery: " + e.getMessage());
        }

        joinDialog = new AlertDialog.Builder(activity)
                .setTitle("Choose a device")
                .setAdapter(listAdapter, (dialog, which) -> {
                    stopDiscovery();
                    connectToDevice(devices.get(which));
                })
                .setOnCancelListener(dialog -> stopDiscovery())
                .setNegativeButton("Cancel", (dialog, which) -> stopDiscovery())
                .create();
        joinDialog.show();
    }

    private void stopDiscovery() {
        if (adapter != null) {
            try {
                adapter.cancelDiscovery();
            } catch (SecurityException ignored) { /* permission already withdrawn */ }
        }
        if (discoveryReceiver != null) {
            try {
                activity.unregisterReceiver(discoveryReceiver);
            } catch (IllegalArgumentException ignored) { /* already unregistered */ }
            discoveryReceiver = null;
        }
        if (joinDialog != null) {
            joinDialog.dismiss();
            joinDialog = null;
        }
    }

    @SuppressWarnings("MissingPermission")
    private void connectToDevice(BluetoothDevice device) {
        connectThread = new ConnectThread(device);
        connectThread.start();
    }

    // Called by MainActivity once the ACTION_REQUEST_DISCOVERABLE prompt returns (any result:
    // hosting still works for already-paired devices even if the player declines)
    void onDiscoverableResult() {
        // Nothing else to do; AcceptThread is already listening.
    }

    // ---- Connection threads (standard Android "Bluetooth Chat" pattern) ----

    @SuppressWarnings("MissingPermission")
    private class AcceptThread extends Thread {
        private final BluetoothServerSocket serverSocket;

        AcceptThread() {
            BluetoothServerSocket socket = null;
            try {
                socket = adapter.listenUsingRfcommWithServiceRecord(SERVICE_NAME, APP_UUID);
            } catch (IOException | SecurityException e) {
                Log.w(TAG, "Could not open server socket: " + e.getMessage());
            }
            serverSocket = socket;
        }

        @Override
        public void run() {
            if (serverSocket == null) return;
            BluetoothSocket socket;
            try {
                socket = serverSocket.accept();
            } catch (IOException e) {
                return;   // cancel() closed the socket, or the connection failed
            }
            try {
                serverSocket.close();
            } catch (IOException ignored) { }
            onConnected(socket);
        }

        void cancel() {
            try {
                if (serverSocket != null) serverSocket.close();
            } catch (IOException ignored) { }
        }
    }

    @SuppressWarnings("MissingPermission")
    private class ConnectThread extends Thread {
        private final BluetoothSocket socket;

        ConnectThread(BluetoothDevice device) {
            BluetoothSocket created = null;
            try {
                created = device.createRfcommSocketToServiceRecord(APP_UUID);
            } catch (IOException | SecurityException e) {
                Log.w(TAG, "Could not create socket: " + e.getMessage());
            }
            socket = created;
        }

        @Override
        public void run() {
            if (socket == null) return;
            try {
                adapter.cancelDiscovery();
                socket.connect();
            } catch (IOException | SecurityException e) {
                Log.w(TAG, "Could not connect: " + e.getMessage());
                try {
                    socket.close();
                } catch (IOException ignored) { }
                return;
            }
            onConnected(socket);
        }

        void cancel() {
            try {
                socket.close();
            } catch (IOException ignored) { }
        }
    }

    private void onConnected(BluetoothSocket socket) {
        connectedThread = new ConnectedThread(socket);
        connectedThread.start();
        connected = true;
    }

    private class ConnectedThread extends Thread {
        private final BluetoothSocket socket;
        private final InputStream input;
        private final OutputStream output;
        private final WriterThread writer;

        // The latest state to send; the writer thread flushes it so a slow/congested
        // link never blocks the caller of write() (the native game loop thread)
        private volatile boolean hasPending = false;
        private volatile float pendingX, pendingY;
        private volatile int pendingDirection, pendingScore;

        ConnectedThread(BluetoothSocket socket) {
            this.socket = socket;
            InputStream in = null;
            OutputStream out = null;
            try {
                in = socket.getInputStream();
                out = socket.getOutputStream();
            } catch (IOException e) {
                Log.w(TAG, "Could not open streams: " + e.getMessage());
            }
            input = in;
            output = out;
            writer = out != null ? new WriterThread() : null;
        }

        @Override
        public void run() {
            if (writer != null) writer.start();
            if (input == null) return;
            BufferedReader reader = new BufferedReader(new InputStreamReader(input, StandardCharsets.UTF_8));
            try {
                String line;
                while ((line = reader.readLine()) != null) {
                    parseLine(line);
                }
            } catch (IOException e) {
                // peer disconnected
            } finally {
                connected = false;
                if (writer != null) writer.interrupt();
                try {
                    socket.close();
                } catch (IOException ignored) { }
            }
        }

        private void parseLine(String line) {
            String[] parts = line.split(",");
            if (parts.length != 4) return;
            try {
                peerX = Float.parseFloat(parts[0]);
                peerY = Float.parseFloat(parts[1]);
                peerDirection = Integer.parseInt(parts[2]);
                peerScore = Integer.parseInt(parts[3]);
            } catch (NumberFormatException ignored) { }
        }

        // Called from the native game loop thread: never blocks on the socket itself
        void write(float x, float y, int direction, int score) {
            if (output == null) return;
            pendingX = x;
            pendingY = y;
            pendingDirection = direction;
            pendingScore = score;
            hasPending = true;
        }

        // Flushes the latest pending state to the socket on its own thread, so a slow
        // or congested Bluetooth link never stalls the caller of write() above
        private class WriterThread extends Thread {
            @Override
            public void run() {
                while (!isInterrupted()) {
                    if (hasPending) {
                        hasPending = false;
                        flush(pendingX, pendingY, pendingDirection, pendingScore);
                    }
                    try {
                        Thread.sleep(16);
                    } catch (InterruptedException e) {
                        return;
                    }
                }
            }
        }

        private void flush(float x, float y, int direction, int score) {
            String line = String.format(Locale.US, "%.1f,%.1f,%d,%d\n", x, y, direction, score);
            try {
                output.write(line.getBytes(StandardCharsets.UTF_8));
            } catch (IOException e) {
                connected = false;
            }
        }

        void cancel() {
            if (writer != null) writer.interrupt();
            try {
                socket.close();
            } catch (IOException ignored) { }
        }
    }
}
