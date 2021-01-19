package jp.mknod.app.ble_monitor;

import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;

import org.altbeacon.beacon.Beacon;
import org.altbeacon.beacon.BeaconConsumer;
import org.altbeacon.beacon.BeaconManager;
import org.altbeacon.beacon.BeaconParser;
import org.altbeacon.beacon.MonitorNotifier;
import org.altbeacon.beacon.RangeNotifier;
import org.altbeacon.beacon.Region;


import java.util.ArrayList;
import java.util.Collection;

public class MainActivity extends AppCompatActivity implements BeaconConsumer {

	private static final int PERMISSION_REQUEST_FINE_LOCATION = 1;
	private static final int PERMISSION_REQUEST_BACKGROUND_LOCATION = 2;

	protected static final String TAG = "MonitoringActivity";
	private BeaconManager beaconManager;
	private static final String IBEACON_FORMAT = "m:2-3=0215,i:4-19,i:20-21,i:22-23,p:24-24";

	ListView mListBle;
	ArrayList<BleBeaconData> list = new ArrayList<BleBeaconData>();
	BleItemAdapter adapter;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mListBle = findViewById(R.id.list_ble);
		adapter = new BleItemAdapter(MainActivity.this,0);
		adapter.setList(list);
		mListBle.setAdapter(adapter);
		mListBle.setOnItemClickListener(new AdapterView.OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

				ListView listView = (ListView)parent;
				BleBeaconData dt = (BleBeaconData)listView.getItemAtPosition(position);

				Log.d("MyActivity", "UUID:" +dt.uuid );

			}
		});


		// BLE
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			if (this.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
				== PackageManager.PERMISSION_GRANTED) {
				if (this.checkSelfPermission(Manifest.permission.ACCESS_BACKGROUND_LOCATION)
					!= PackageManager.PERMISSION_GRANTED) {
					if (this.shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_BACKGROUND_LOCATION)) {
						final AlertDialog.Builder builder = new AlertDialog.Builder(this);
						builder.setTitle("This app needs background location access");
						builder.setMessage("Please grant location access so this app can detect beacons in the background.");
						builder.setPositiveButton(android.R.string.ok, null);
						builder.setOnDismissListener(new DialogInterface.OnDismissListener() {

							@TargetApi(23)
							@Override
							public void onDismiss(DialogInterface dialog) {
								requestPermissions(new String[]{Manifest.permission.ACCESS_BACKGROUND_LOCATION},
									PERMISSION_REQUEST_BACKGROUND_LOCATION);
							}

						});
						builder.show();
					}
					else {
						final AlertDialog.Builder builder = new AlertDialog.Builder(this);
						builder.setTitle("Functionality limited");
						builder.setMessage("Since background location access has not been granted, this app will not be able to discover beacons in the background.  Please go to Settings -> Applications -> Permissions and grant background location access to this app.");
						builder.setPositiveButton(android.R.string.ok, null);
						builder.setOnDismissListener(new DialogInterface.OnDismissListener() {

							@Override
							public void onDismiss(DialogInterface dialog) {
							}

						});
						builder.show();
					}

				}
			} else {
				if ( this.shouldShowRequestPermissionRationale(Manifest.permission.ACCESS_FINE_LOCATION) == false) {

					requestPermissions(new String[]{
							Manifest.permission.ACCESS_FINE_LOCATION,
							Manifest.permission.ACCESS_BACKGROUND_LOCATION},
						PERMISSION_REQUEST_FINE_LOCATION);
				}
				else {
					final AlertDialog.Builder builder = new AlertDialog.Builder(this);
					builder.setTitle("Functionality limited");
					builder.setMessage("Since location access has not been granted, this app will not be able to discover beacons.  Please go to Settings -> Applications -> Permissions and grant location access to this app.");
					builder.setPositiveButton(android.R.string.ok, null);
					builder.setOnDismissListener(new DialogInterface.OnDismissListener() {

						@Override
						public void onDismiss(DialogInterface dialog) {
						}

					});
					builder.show();
				}

			}
		}

		beaconManager = BeaconManager.getInstanceForApplication(this);
		// To detect proprietary beacons, you must add a line like below corresponding to your beacon
		// type.  Do a web search for "setBeaconLayout" to get the proper expression.
		// beaconManager.getBeaconParsers().add(new BeaconParser().
		//        setBeaconLayout("m:2-3=beac,i:4-19,i:20-21,i:22-23,p:24-24,d:25-25"));
//		beaconManager.bind(this);
		beaconManager.getBeaconParsers().add(new BeaconParser().setBeaconLayout(IBEACON_FORMAT));

	}
	@Override
	protected void onResume() {

		super.onResume();
		beaconManager.bind(this);
		//サービス開始
	}

	@Override
	protected void onPause() {
		super.onPause();
		beaconManager.unbind(this);
		//サービス終了
	}

	@Override
	public void onBeaconServiceConnect() {
		beaconManager.removeAllRangeNotifiers();
		beaconManager.addRangeNotifier(new RangeNotifier() {
			@Override
			public void didRangeBeaconsInRegion(Collection<Beacon> beacons, Region region) {

				list.clear();

				for(Beacon beacon: beacons) {

					BleBeaconData dt = new BleBeaconData();
					dt.uuid =  beacon.getId1().toString();
					dt.major = beacon.getId2().toHexString();
					dt.minor =  beacon.getId3().toHexString();
					dt.rssi =  beacon.getRssi();
					dt.distance =  beacon.getDistance();

//					"01020304-0506-0708-0102-030405060708"
					list.add(dt);

					Log.d("MyActivity", "UUID:" + beacon.getId1() + ", major:"
						+ beacon.getId2() + ", minor:" + beacon.getId3() + ", RSSI:"
						+ beacon.getRssi() + ", TxPower:" + beacon.getTxPower()
						+ ", Distance:" + beacon.getDistance());
				}

				if (beacons.size() > 0) {
					adapter.setList(list);
					adapter.notifyDataSetChanged();
				}
//				if (beacons.size() > 0) {
//					Log.i(TAG, "The first beacon I see is about "+beacons.iterator().next().getDistance()+" meters away.");
//				}
			}
		});

		try {
			beaconManager.startRangingBeaconsInRegion(new Region("myRangingUniqueId", null, null, null));
		} catch (RemoteException e) {    }
	}

//	@Override
//	protected void onDestroy() {
//		super.onDestroy();
//		beaconManager.unbind(this);
//	}
//
//	@Override
//	public void onBeaconServiceConnect() {
//		beaconManager.removeAllMonitorNotifiers();
//		beaconManager.addMonitorNotifier(new MonitorNotifier() {
//			@Override
//			public void didEnterRegion(Region region) {
//				Log.i(TAG, "I just saw an beacon for the first time!");
//			}
//
//			@Override
//			public void didExitRegion(Region region) {
//				Log.i(TAG, "I no longer see an beacon");
//			}
//
//			@Override
//			public void didDetermineStateForRegion(int state, Region region) {
//				Log.i(TAG, "I have just switched from seeing/not seeing beacons: "+state);
//			}
//		});
//
//		try {
//			beaconManager.startMonitoringBeaconsInRegion(new Region("myMonitoringUniqueId", null, null, null));
//		} catch (RemoteException e) {    }
//	}
}
