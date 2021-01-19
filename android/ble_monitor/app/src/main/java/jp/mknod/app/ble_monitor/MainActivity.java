package jp.mknod.app.ble_monitor;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;


import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanSettings;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.nio.ByteBuffer;
import java.nio.channels.CompletionHandler;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;


public class MainActivity extends AppCompatActivity {

	Context mContext;
	String TAG = "MainActivity";
	ListView mListBle;
	Button bt_scan;
	int REQUEST_ENABLE_BT = 1;

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mContext = this;

		bluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
		bluetoothAdapter = bluetoothManager.getAdapter();

		if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {

			Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);

		}else {

			bleDeviceListAdapter = new BleDeviceListAdapter(this, 0);

			bt_scan = findViewById(R.id.bt_scan);
			bt_scan.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					scanBleDevice();
				}
			});

			mListBle = findViewById(R.id.list_ble);
			mListBle.setAdapter(bleDeviceListAdapter);
			mListBle.setOnItemClickListener(new AdapterView.OnItemClickListener() {

				@Override
				public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

					ListView listView = (ListView) parent;
					BleDeviceData data = (BleDeviceData) listView.getItemAtPosition(position);

					Log.d("MyActivity", "ADDR:" + data.device.getAddress());

					if (connectionState == STATE_DISCONNECTED) {
						bluetoothGatt = data.device.connectGatt(mContext, false, gattCallback);
					} else if (connectionState == STATE_CONNECTED) {
						bluetoothGatt.disconnect();
					}
				}
			});

			scanBleDevice();
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (requestCode == REQUEST_ENABLE_BT) {
			if (resultCode == RESULT_OK) {
				// "Bluetooth が有効化された"
			}
		}
	}

	void scanBleDevice() {

		bluetoothAdapter.getBluetoothLeScanner().startScan(scanCallback);
	}

	void refreshList(){

	  new Thread(new Runnable() {
		  @Override
		  public void run() {
			  runOnUiThread(new Runnable(){
				  @Override
				  public void run(){
					  bleDeviceListAdapter.notifyDataSetChanged();
				  }
			  });
		  }
	  }).start();
  }

	//
	// BLE 端末の検出
	//
	private static final long SCAN_PERIOD = 10000;
	private BluetoothManager bluetoothManager;
	private BluetoothAdapter bluetoothAdapter;
	private BleDeviceListAdapter bleDeviceListAdapter;

	ScanCallback scanCallback = new ScanCallback() {

		@Override
		public void onScanResult(int callbackType, ScanResult result) {

			Log.i(TAG, "BLE// onScanResult");
			Log.i("callbackType", String.valueOf(callbackType));
			Log.i("result", result.toString());

			BluetoothDevice device = result.getDevice();
			if( bleDeviceListAdapter.addDevice(device) ){
				refreshList();
			}

			if( device.getName()!=null && device.getName().equals("mknod")){

				if(connectionState == STATE_DISCONNECTED) {

					connectionState = STATE_CONNECTING;
					bluetoothGatt = device.connectGatt(mContext, false, gattCallback);
				}
			}
		}

		@Override
		public void onBatchScanResults(List<ScanResult> results) {
			System.out.println("BLE// onBatchScanResults");
			for (ScanResult sr : results) {
				Log.i("ScanResult - Results", sr.toString());
			}
		}

		@Override
		public void onScanFailed(int errorCode) {
			System.out.println("BLE// onScanFailed");
			Log.e("Scan Failed", "Error Code: " + errorCode);
		}
	};


  //
	//  GATT Peripheral(Server)へ接続
	//
	private BluetoothGatt bluetoothGatt;

	private int connectionState = STATE_DISCONNECTED;

	private static final int STATE_DISCONNECTED = 0;
	private static final int STATE_CONNECTING = 1;
	private static final int STATE_CONNECTED = 2;

		@Override
	public void onDestroy() {

		super.onDestroy();

		if (bluetoothGatt == null) {
			return;
		}
		bluetoothGatt.close();
		bluetoothGatt = null;
	}

	private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

		@Override
		public void onConnectionStateChange(BluetoothGatt gatt, int status,
		                                    int newState) {

			if (newState == BluetoothProfile.STATE_CONNECTED) {

				connectionState = STATE_CONNECTED;
				Log.i(TAG, "Connected to GATT server.");
				Log.i(TAG, "Attempting to start service discovery:" +

				bluetoothGatt.discoverServices());

				BleDeviceData dt = bleDeviceListAdapter.search( gatt.getDevice());
				dt.connected = true;
				refreshList();

			} else if (newState == BluetoothProfile.STATE_DISCONNECTED) {

				connectionState = STATE_DISCONNECTED;
				Log.i(TAG, "Disconnected from GATT server.");

				BleDeviceData dt = bleDeviceListAdapter.search( gatt.getDevice());
				dt.connected = false;
				refreshList();

			}else{
			}
		}

		@Override
		// New services discovered
		public void onServicesDiscovered(BluetoothGatt gatt, int status) {

			Log.i(TAG, "onServicesDiscovered");

			if (status == BluetoothGatt.GATT_SUCCESS) {

//				gattServices = gatt.getServices();
//				displayGattServices(gattServices);

				uart_rx_enable();

			} else {
				Log.w(TAG, "onServicesDiscovered received: " + status);
			}
		}

		@Override
		public void onCharacteristicChanged( BluetoothGatt gatt, BluetoothGattCharacteristic characteristic )
		{
			Log.i(TAG, "onCharacteristicChanged");

			// RX
			if( kUartRxCharacteristicUUID.equals( characteristic.getUuid() ) )
			{
				byte[] bytes = characteristic.getValue();
				String data = new String(bytes);
				Log.i(TAG, "val:" + data);

				try {
					JSONObject json = new JSONObject(data);
					double t = json.optDouble("temp");

					BleDeviceData dt = bleDeviceListAdapter.search( gatt.getDevice());
					dt.temp = t;
					refreshList();

				} catch (JSONException e) {
					e.printStackTrace();
				}
			}
		}

		@Override
		// Result of a characteristic read operation
		public void onCharacteristicRead(BluetoothGatt gatt,
		                                 BluetoothGattCharacteristic characteristic,
		                                 int status) {

			Log.i(TAG, "onCharacteristicRead");
		}

	};


	List<BluetoothGattService> gattServices;
	ArrayList<HashMap<String, String>> gattServiceData = new ArrayList<HashMap<String, String>>();
	ArrayList<ArrayList<BluetoothGattCharacteristic>> mGattCharacteristics;
	String LIST_NAME = "NAME";
	String LIST_UUID = "UUID";

	private void displayGattServices(List<BluetoothGattService> gattServices) {

		if (gattServices == null) return;

		String uuid = null;

		ArrayList<HashMap<String, String>> gattServiceData = new ArrayList<HashMap<String, String>>();
		ArrayList<ArrayList<HashMap<String, String>>> gattCharacteristicData = new ArrayList<ArrayList<HashMap<String, String>>>();

		mGattCharacteristics = new ArrayList<ArrayList<BluetoothGattCharacteristic>>();

		// Loops through available GATT Services.
		for (BluetoothGattService gattService : gattServices) {

			HashMap<String, String> currentServiceData = new HashMap<String, String>();
			uuid = gattService.getUuid().toString();
			Log.i(TAG, "SERVICE: " + uuid);

//				currentServiceData.put(	LIST_NAME, SampleGattAttributes.lookup(uuid, unknownServiceString));
//				currentServiceData.put(LIST_UUID, uuid);
//				gattServiceData.add(currentServiceData);

			ArrayList<HashMap<String, String>> gattCharacteristicGroupData = new ArrayList<HashMap<String, String>>();
			List<BluetoothGattCharacteristic> gattCharacteristics = gattService.getCharacteristics();
			ArrayList<BluetoothGattCharacteristic> charas = new ArrayList<BluetoothGattCharacteristic>();
			// Loops through available Characteristics.
			for (BluetoothGattCharacteristic gattCharacteristic : gattCharacteristics) {

				charas.add(gattCharacteristic);
				HashMap<String, String> currentCharaData = new HashMap<String, String>();

				uuid = gattCharacteristic.getUuid().toString();
//					currentCharaData.put(
//						LIST_NAME, SampleGattAttributes.lookup(uuid,
//							unknownCharaString));
//					currentCharaData.put(LIST_UUID, uuid);
				Log.i(TAG, "CHARACTERISTIC: " + uuid);

				gattCharacteristicGroupData.add(currentCharaData);
			}

			mGattCharacteristics.add(charas);
			gattCharacteristicData.add(gattCharacteristicGroupData);
		}
	}


	private static final UUID kUartServiceUUID = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
	private static final UUID kUartTxCharacteristicUUID = UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
	private static final UUID kUartRxCharacteristicUUID = UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e");

	public static UUID kClientCharacteristicConfigUUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

	void uart_rx_enable() {

		BluetoothGattService uart_service = bluetoothGatt.getService(kUartServiceUUID);

		if(uart_service != null){

			BluetoothGattCharacteristic rx = uart_service.getCharacteristic(kUartRxCharacteristicUUID);
			bluetoothGatt.setCharacteristicNotification(rx, true);

			BluetoothGattDescriptor descriptor = rx.getDescriptor(kClientCharacteristicConfigUUID);
			descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
			bluetoothGatt.writeDescriptor(descriptor);
		}
	}

	void send(){

		byte[] val = new byte[8];
		val[0]=0x31;
		val[1]=0x32;
		val[2]=0x33;
		val[3]=0x34;
		val[4]=0x35;
		val[5]=0x36;
		val[6]=0x37;

		BluetoothGattCharacteristic blechar = bluetoothGatt.getService( kUartServiceUUID ).getCharacteristic( kUartTxCharacteristicUUID );
		blechar.setValue(val);

		if( bluetoothGatt.writeCharacteristic(blechar) ){
			Log.i(TAG, "write(): success");
		}else{
			Log.i(TAG, "write(): failed");
		}
	}

}

