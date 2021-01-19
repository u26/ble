package jp.mknod.app.ble_monitor;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import androidx.annotation.NonNull;

import java.util.ArrayList;

public class BleItemAdapter extends ArrayAdapter<BleBeaconData> {

	Context context;
	ArrayList<BleBeaconData> data;
	LayoutInflater layoutInflater;

	public BleItemAdapter(@NonNull Context context, int resource) {
		super(context, resource);
		this.context = context;
		this.layoutInflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

	}

	public void updateData(){

	}

	public void setList(ArrayList<BleBeaconData> list) {
		this.data = list;
	}

	@Override
	public int getCount() {
		return data.size();
	}

	@Override
	public BleBeaconData getItem(int position) {
		return data.get(position);
	}


	@Override
	public View getView(int position, View convertView, ViewGroup parent) {

		convertView = layoutInflater.inflate(R.layout.ble_item, parent, false);

		((TextView) convertView.findViewById(R.id.tx_uuid)).setText(data.get(position).uuid);
		((TextView) convertView.findViewById(R.id.tx_name)).setText(data.get(position).major);
		((TextView) convertView.findViewById(R.id.tx_temp)).setText(data.get(position).major);

//		String tmp = data.get(position).minor.substring(2,4);
//		String hum = data.get(position).minor.substring(4,6);
//
//		int i_tmp = get8bitData(tmp);
//		int i_hum = get8bitData(hum);
//
////		((TextView) convertView.findViewById(R.id.tx_minor)).setText(String.valueOf(i_tmp) + " " + i_hum);
//		((TextView) convertView.findViewById(R.id.tx_minor)).setText(tmp + " " + hum);
//		((TextView) convertView.findViewById(R.id.tx_rssi)).setText(String.valueOf(data.get(position).rssi));
//		((TextView) convertView.findViewById(R.id.tx_dist)).setText(String.format("%.3f", data.get(position).distance));

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
