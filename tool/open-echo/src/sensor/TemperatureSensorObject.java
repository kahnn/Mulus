package sensor;

import java.io.IOException;

import com.sonycsl.echo.eoj.device.sensor.TemperatureSensor;

public class TemperatureSensorObject extends TemperatureSensor {

	byte[] mStatus = {0x30};
	byte[] mLocation = {0x00};
	byte[] mVersion = {0x01, 0x01, 0x61, 0x00};
	byte[] mFaultStatus = {0x42};
	byte[] mManufacturerCode = {0,0,0};

	byte[] mTemperature = {0x00,0x10};

	public TemperatureSensorObject() {
		// default;
		super();
	}

	@Override
	protected void setupPropertyMaps() {
		super.setupPropertyMaps();

		// RE-ADD SetProperty OperationStatus
		addSetProperty(EPC_OPERATION_STATUS);
		// ADD Announce
		addStatusChangeAnnouncementProperty(EPC_MEASURED_TEMPERATURE_VALUE);
	}

	@Override
	protected boolean setOperationStatus(byte[] edt) {
		changeOperationStatus(edt[0] == 0x30);
		return true;
	}

	@Override
	protected byte[] getOperationStatus() { return mStatus; }
	
	public void changeOperationStatus(boolean status) {
		byte b = (status?(byte)0x30:(byte)0x31) ;
		
		if(mStatus[0] == b) return ;
		mStatus[0] = b;
		try {
			inform().reqInformOperationStatus().send();
		} catch (IOException e) {e.printStackTrace();}
	}

	@Override
	protected byte[] getMeasuredTemperatureValue() {
		return  mTemperature;
	}

	public void changeMeasuredTemperatureValue(short temp) {
		mTemperature[0] = (byte)((temp >> 8) & 0xff);
		mTemperature[1] = (byte)((temp) & 0xff);
		try {
			inform().reqInformMeasuredTemperatureValue().send();
		} catch (IOException e) {e.printStackTrace();}
	}

	@Override
	protected boolean setInstallationLocation(byte[] edt) {
		changeInstallationLocation(edt[0]);
		return true;
	}
	
	@Override
	protected byte[] getInstallationLocation() {return mLocation;}
	
	public void changeInstallationLocation(byte location) {
		if(mLocation[0] == location) return ;
		mLocation[0] = location;
		try {
			inform().reqInformInstallationLocation().send() ;
		} catch (IOException e) {e.printStackTrace();}
	}

	@Override
	protected byte[] getFaultStatus() {
		return mFaultStatus;
	}

	@Override
	protected byte[] getManufacturerCode() {
		return mManufacturerCode;
	}
}
