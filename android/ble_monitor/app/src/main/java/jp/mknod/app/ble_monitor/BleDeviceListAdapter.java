package jp.mknod.app.ble_monitor;

import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;

import java.util.ArrayList;

public class BleDeviceListAdapter extends ArrayAdapter<BleDeviceData> {

	Context context;
	ArrayList<BleDeviceData> data ;
	LayoutInflater layoutInflater;

	public BleDeviceListAdapter(@NonNull Context context, int resource) {
		super(context, resource);
		this.context = context;
		this.layoutInflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		this.data = new ArrayList<BleDeviceData>();
	}

	public void setList(ArrayList<BleDeviceData> list) {

		this.data = list;
	}

	public BleDeviceData search(BluetoothDevice dt) {

		for (int i = 0; i < data.size(); i++) {
			if (data.get(i).device.getAddress().equals(dt.getAddress())) {
				return data.get(i);
			}
		}
		return null;
	}

	public boolean addDevice( BluetoothDevice device ) {

		boolean exist = false;
		for (int i = 0; i < data.size(); i++) {

			if (data.get(i).device.getAddress().equals(device.getAddress())) {
				exist = true;
			}
		}

		if (!exist) {

			BleDeviceData dev = new BleDeviceData();
			dev.device = device;
			this.data.add( dev );
			return true;

		}else{

			return false;
		}
	}

	@Override
	public int getCount() {

		return data.size();
	}

	@Override
	public BleDeviceData getItem(int position) {

		return data.get(position);
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {

		convertView = layoutInflater.inflate(R.layout.ble_item, parent, false);

		((TextView) convertView.findViewById(R.id.tx_uuid)).setText(data.get(position).device.getAddress());
		((TextView) convertView.findViewById(R.id.tx_name)).setText(data.get(position).device.getName());
		((TextView) convertView.findViewById(R.id.tx_temp)).setText(String.format("%.2f",data.get(position).temp));

		if( data.get(position).connected ){
			((TextView) convertView.findViewById(R.id.tx_connected)).setText("CONNECTED");
		}else{
			((TextView) convertView.findViewById(R.id.tx_connected)).setText("");
		}

		/*
		((TextView) convertView.findViewById(R.id.tx_uuid)).setText(data.get(position).uuid);
		((TextView) convertView.findViewById(R.id.tx_major)).setText(data.get(position).major);

		String tmp = data.get(position).minor.substring(2,4);
		String hum = data.get(position).minor.substring(4,6);

		int i_tmp = get8bitData(tmp);
		int i_hum = get8bitData(hum);

		((TextView) convertView.findViewById(R.id.tx_minor)).setText(tmp + " " + hum);
		((TextView) convertView.findViewById(R.id.tx_rssi)).setText(String.valueOf(data.get(position).rssi));
		((TextView) convertView.findViewById(R.id.tx_dist)).setText(String.format("%.3f", data.get(position).distance));
*/
		return convertView;
	}

//	public static int get8bitData(String hex) {
//
//		int val;
//		val = Integer.parseInt(hex, 16);
//		if( val >= 128 ){
//			val = val - 256;
//		}
//		return val;
//	}
}
