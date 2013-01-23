package sensor;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import sensor.TemperatureSensorObject;

import com.sonycsl.echo.Echo;
import com.sonycsl.echo.EchoProperty;
import com.sonycsl.echo.EchoUtils;
import com.sonycsl.echo.eoj.EchoObject;
import com.sonycsl.echo.eoj.EchoObject.Receiver;
import com.sonycsl.echo.eoj.device.DeviceObject;
import com.sonycsl.echo.eoj.device.sensor.TemperatureSensor;
import com.sonycsl.echo.eoj.profile.NodeProfile;
import com.sonycsl.echo.node.EchoNode;

public class SampleTemperatureSensor {

	private void dumpNodes(EchoNode nodes[])
	{
		for (int i = 0; i < nodes.length; i++) {
			EchoNode node = nodes[i];
			System.out.println("Node[" + i + "] : " + node + " : " + node.getAddress());
			DeviceObject devs[] = node.getDevices();
			for (int j = 0; j < devs.length; j++) {
				DeviceObject dev = devs[j];
				System.out.println("Device[" + j + "] : " + dev + 
						" : " + dev.isProxy() + 
						" : " + EchoUtils.toHexString(dev.getGetProperties()) +
						" : " + EchoUtils.toHexString(dev.getSetProperties()));
			}
		}
	}
	
	private void updateDomain()
		throws Exception
	{
		NodeProfile.informG().reqInformInstanceListNotification().send();
	}

	private byte[] getProperty(EchoNode nodes[], int nindex, int oindex, int epc)
			throws Exception
	{
		EchoNode node = nodes[nindex];
		DeviceObject dev = node.getDevices()[oindex];
//		if (dev instanceof TemperatureSensorObject && TemperatureSensor.EPC_MEASURED_TEMPERATURE_VALUE == (byte)(epc & 0xff)) {
//			// return local value
//			return ((TemperatureSensorObject) dev).getMeasuredTemperatureValue();
//		}

		class MyReceiver extends Receiver {
			EchoProperty prop = null;
			@Override
			synchronized protected boolean onGetProperty(EchoObject eoj, short tid, byte esv, EchoProperty property, boolean success) {
				System.out.println("onGetProperty: " + eoj + ":" + esv + ":" + property + ":" + success);
				prop = property;
				notifyAll();
				return true;
			}
		};
		MyReceiver receiver = new MyReceiver();
		synchronized (receiver) {
			dev.setReceiver(receiver);
			int retry = 0;
			while (null == receiver.prop && retry < 3) {
				dev.get().reqGetProperty((byte)(epc & 0xff)).send();
				try {
					receiver.wait(3 * 1000);
			    } catch (InterruptedException e) {
			    }
				retry++;
			}
		}
		return (null == receiver.prop) ? null : receiver.prop.edt;
	}

	private void setProperty(EchoNode nodes[], int nindex, int oindex, int epc, byte data)
			throws Exception
	{
		EchoNode node = nodes[nindex];
		DeviceObject dev = node.getDevices()[oindex];
		dev.set().reqSetProperty((byte)(epc & 0xff), new byte[]{data}).send();
	}

	void run(String[] args)
		throws Exception
	{
		TemperatureSensorObject tempObj = new TemperatureSensorObject();
		DeviceObject[] devices = { tempObj };
		NodeProfile profile = new NodeProfileObject();
		
		System.out.println("LocalIP : " + EchoUtils.getLocalIpAddress());

		// XXX debug
		Echo.addEventListener(new Echo.Logger(System.out));
		Echo.start(profile, devices);
		
		{
			BufferedReader stdReader =
		    	  new BufferedReader(new InputStreamReader(System.in));
		    System.out.print("IN>> ");
		    String line;
		    while ((line = stdReader.readLine()) != null) {
		    	if (line.startsWith("pao")) {
		    		EchoNode myNode = Echo.getNode();
		    		System.out.println("My Node : " + myNode);
		    		EchoNode nodes[] = Echo.getNodes();
		    		dumpNodes(nodes);
		    	} else if (line.startsWith("upd")) {
		    		updateDomain();
		    	} else if (line.startsWith("gp")) {
		    		String avs[] = line.split(" ");
		    		EchoNode nodes[] = Echo.getNodes();
		    		int epc = Integer.parseInt(avs[3], 16);
		    		byte[] data = getProperty(nodes, 
		    				Integer.parseInt(avs[1]), 
		    				Integer.parseInt(avs[2]),
		    				epc);
		    		System.out.println("Get Property: "
		    				+ EchoUtils.toHexString((byte)(epc & 0xff)) 
		    				+ "="
		    				+ EchoUtils.toHexString(data));
		    	} else if (line.startsWith("sp")) {
		    		String avs[] = line.split(" ");
		    		EchoNode nodes[] = Echo.getNodes();
		    		int epc = Integer.parseInt(avs[3], 16);
		    		setProperty(nodes, 
		    				Integer.parseInt(avs[1]), 
		    				Integer.parseInt(avs[2]),
		    				epc,
		    				Byte.parseByte(avs[4], 16));
		    		System.out.println("Set Property: " + EchoUtils.toHexString((byte)(epc & 0xff)));
		    	} else if (line.startsWith("ct")) {
		    		String avs[] = line.split(" ");
		    		short temp = Short.parseShort(avs[1]);
		    		tempObj.changeMeasuredTemperatureValue(temp);
		    	} else if (line.startsWith("help")) {
			    		System.out.println("help");
			    		System.out.println("pao                               : print all object");
			    		System.out.println("upd                               : update domain information");
			    		System.out.println("gp node deoj prop-code            : get property");
			    		System.out.println("sp node deoj prop-code prop-value : set property");
			    		System.out.println("ct temperature                    : change temperature");
		    	} else {
		    		; // ignore
		    	}
		    	System.out.print("\nIN>> ");
		    }
		    stdReader.close();
		    System.out.println("\n>>EXIT");
		}
		Echo.stop();
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args)
		throws Exception
	{
		SampleTemperatureSensor obj = new SampleTemperatureSensor();
		obj.run(args);
	}
}
