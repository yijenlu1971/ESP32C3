using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;

namespace BluetoothLE
{
    public partial class Form1 : Form
    {
        // Create Bluetooth Listener (Only searching for BLE devices)
        private BluetoothLEAdvertisementWatcher watcher = new BluetoothLEAdvertisementWatcher();
        private List<BLEDevice> KnownDevices = new List<BLEDevice>();
        private List<BLEDevice> UnknownDevices = new List<BLEDevice>();
        private bool bScan = false;
        private delegate void SafeAddBleList(ulong addr, string name);

        private BluetoothLEDevice btLeDevice = null;
        private List<GattDeviceService> DevServices = new List<GattDeviceService>();
        private List<GattCharacteristic> ServCharact = new List<GattCharacteristic>();
        public Form1()
        {
            InitializeComponent();
            ConnBtn.Enabled = ServiceBtn.Enabled = false;
        }

        // Add BLE devices to list
        void AddBleList(ulong addr, string name)
        {
            if (ResultList.InvokeRequired)   // 參考 Microsoft Invoke 做法 (不安全的跨執行緒呼叫)
            {
                var d = new SafeAddBleList(AddBleList);
                ResultList.Invoke(d, new object[] { addr, name });
            }
            else
            {
                ResultList.Items.Add(String.Format("Addr: {0:X}   Name: {1}", addr, name));
            }
        }
        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            foreach (BLEDevice info in KnownDevices)
            {
                if (info.DevAddr == eventArgs.BluetoothAddress) return;
            }
            foreach (BLEDevice info in UnknownDevices)
            {
                if (info.DevAddr == eventArgs.BluetoothAddress) return;
            }

            // Tell the user we see an advertisement and print some properties
            BLEDevice devInfo = new BLEDevice();
            devInfo.DevAddr = eventArgs.BluetoothAddress;

            Console.WriteLine(String.Format("Advertisement:"));
            Console.WriteLine(String.Format("  BT_ADDR: {0:X}", eventArgs.BluetoothAddress));
            Console.WriteLine(String.Format("  BT_TYPE: {0}", eventArgs.BluetoothAddressType));
            if (eventArgs.Advertisement.LocalName.Length > 0)
            {
                Console.WriteLine(String.Format("  AD_NAME: {0}", eventArgs.Advertisement.LocalName));
                devInfo.DevName = eventArgs.Advertisement.LocalName;
            }
            Console.WriteLine(String.Format("  AD_TYPE: {0}", eventArgs.AdvertisementType));
            if (eventArgs.Advertisement.Flags != null) Console.WriteLine(String.Format("  AD_FLAG: {0}", eventArgs.Advertisement.Flags));

            if (eventArgs.AdvertisementType <= BluetoothLEAdvertisementType.ConnectableDirected)
            {
                // BLEAdvertisementData 
                for (int i = 0; i < eventArgs.Advertisement.DataSections.Count; i++)
                {
                    var data = eventArgs.Advertisement.DataSections[i];
                    using (var dataReader = Windows.Storage.Streams.DataReader.FromBuffer(data.Data))
                    {
                        byte[] byteArray = new byte[data.Data.Length];
                        dataReader.UnicodeEncoding = Windows.Storage.Streams.UnicodeEncoding.Utf8;
                        dataReader.ReadBytes(byteArray);
                        if (data.DataType == BluetoothLEAdvertisementDataTypes.CompleteLocalName)
                        {
                            string str = Encoding.Default.GetString(byteArray);
                            Console.WriteLine(String.Format("[{0}]DATA: {1}", i, str));
                            devInfo.DevName = str;
                        }
                        else
                        {
                            Console.Write(String.Format("[{0}]DATA: ", i));
                            foreach (byte val in byteArray) Console.Write(String.Format("{0:X2}", val));
                            Console.WriteLine();
                        }
                    }
                    Console.WriteLine(String.Format("[{0}]TYPE: {1}", i, data.DataType));
                }

                // show Service UUID
                devInfo.ServiceUuids = eventArgs.Advertisement.ServiceUuids;
                for (int i = 0; i < eventArgs.Advertisement.ServiceUuids.Count; i++)
                {
                    var data = eventArgs.Advertisement.ServiceUuids[i];
                    Console.WriteLine(String.Format("[{0}]UUID: {1:X}", i, data.ToString()));
                }

                KnownDevices.Add(devInfo);
                AddBleList(devInfo.DevAddr, devInfo.DevName);
            }
            else
            {
                UnknownDevices.Add(devInfo);
            }
            Console.WriteLine();
        }
        private void DiscoverBtn_Click(object sender, System.EventArgs e)
        {
            if (!bScan)
            {
                KnownDevices.Clear();
                UnknownDevices.Clear();
                ResultList.Items.Clear();
                ServiceList.Items.Clear();
                CharList.Items.Clear();
                DescList.Items.Clear();
                ConnBtn.Enabled = ServiceBtn.Enabled = false;
                ConnBtn.Text = "連線BLE";

                btLeDevice?.Dispose();
                btLeDevice = null;

                watcher.ScanningMode = BluetoothLEScanningMode.Active;
                // Only activate the watcher when we're recieving values >= -80
                watcher.SignalStrengthFilter.InRangeThresholdInDBm = -80;
                // Stop watching if the value drops below -90 (user walked away)
                watcher.SignalStrengthFilter.OutOfRangeThresholdInDBm = -90;
                // Register callback for when we see an advertisements
                watcher.Received += OnAdvertisementReceived;

                // Wait 5 seconds to make sure the device is really out of range
                watcher.SignalStrengthFilter.OutOfRangeTimeout = TimeSpan.FromMilliseconds(5000);
                watcher.SignalStrengthFilter.SamplingInterval = TimeSpan.FromMilliseconds(2000);

                // Starting watching for advertisements
                watcher.Start();
                DiscoverBtn.Text = "停止";
            }
            else
            {
                watcher.Stop();
                DiscoverBtn.Text = "列舉BLE";
            }
            bScan = !bScan;
        }

        private void ResultList_SelectedIndexChanged(object sender, EventArgs e)
        {
            ConnBtn.Enabled = (ResultList.SelectedIndex != -1) ? true : false;
        }

        private async void ConnBtn_Click(object sender, EventArgs e)
        {
            ConnBtn.Enabled = false;
            if (bScan)
            {
                bScan = false;
                watcher.Stop();
                DiscoverBtn.Text = "列舉BLE";
            }

            if (btLeDevice != null)
            {
                btLeDevice.Dispose();
                btLeDevice = null;
                ConnBtn.Text = "連線BLE";
                ServiceBtn.Enabled = false;
            }
            else
            {
                try
                {
                    BLEDevice dev = KnownDevices[ResultList.SelectedIndex];
                    btLeDevice = await BluetoothLEDevice.FromBluetoothAddressAsync(dev.DevAddr);

                    if (btLeDevice != null)
                    {
                        ConnBtn.Text = "斷線BLE";
                        ServiceBtn.Enabled = true;
                        Console.WriteLine("連線狀態:{0}", btLeDevice.ConnectionStatus);
                    }
                }
                catch (Exception ex) { Console.WriteLine("BLE_Service exception: ", ex.Message); }
            }
            ConnBtn.Enabled = true;
        }

        private async void ServiceBtn_Click(object sender, EventArgs e)
        {
            if (btLeDevice != null)
            {
                ServiceBtn.Enabled = false;
                ServiceList.Items.Clear();
                // Note: BluetoothLEDevice.GattServices property will return an empty list for unpaired devices. For all uses we recommend using the GetGattServicesAsync method.
                // BT_Code: GetGattServicesAsync returns a list of all the supported services of the device (even if it's not paired to the system).
                // If the services supported by the device are expected to change during BT usage, subscribe to the GattServicesChanged event.
                GattDeviceServicesResult result = await btLeDevice.GetGattServicesAsync(BluetoothCacheMode.Uncached);

                if (result.Status == GattCommunicationStatus.Success)
                {
                    DevServices.Clear();
                    var services = result.Services;
                    Console.WriteLine(String.Format("Found {0} services", services.Count));
                    foreach (var service in services)
                    {
                        DevServices.Add(service);
                        ServiceList.Items.Add(DisplayHelpers.GetServiceName(service));
                        Console.WriteLine(String.Format("UUID: {0:X}", service.Uuid.ToString()));
                    }
                }
                else
                {
                    Console.WriteLine("Device unreachable");
                }

                Console.WriteLine("連線狀態:{0}", btLeDevice.ConnectionStatus);
                ServiceBtn.Enabled = true;
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            btLeDevice?.Dispose();
            btLeDevice = null;
        }

        private async void ServiceList_SelectedIndexChanged(object sender, EventArgs e)
        {
            var service = DevServices[ServiceList.SelectedIndex];
            IReadOnlyList<GattCharacteristic> characteristics = null;

            try
            {
                var accessStatus = await service.RequestAccessAsync();
                if (accessStatus == DeviceAccessStatus.Allowed)
                {
                    // BT_Code: Get all the child characteristics of a service. Use the cache mode to specify uncached characterstics only 
                    // and the new Async functions to get the characteristics of unpaired devices as well. 
                    var result = await service.GetCharacteristicsAsync(BluetoothCacheMode.Uncached);
                    if (result.Status == GattCommunicationStatus.Success)
                    {
                        CharList.Items.Clear();
                        ServCharact.Clear();
                        characteristics = result.Characteristics;
                        foreach (GattCharacteristic c in characteristics)
                        {
                            ServCharact.Add(c);
                            DescList.Items.Clear();
                            CharList.Items.Add(DisplayHelpers.GetCharacteristicName(c));
                            Console.WriteLine("Charact:{0}", DisplayHelpers.GetCharacteristicName(c));
                        }
                    }
                    else
                    {
                        Console.WriteLine("GetCharacteristics {0}", result.Status);
                    }
                }
                else
                {
                    Console.WriteLine("Error accessing service.");
                }
            }
            catch (Exception ex) { Console.WriteLine("BLE_Charact exception: ", ex.Message); }
        }

        private async void CharList_SelectedIndexChanged(object sender, EventArgs e)
        {
            GattCharacteristic selectedCharacteristic = ServCharact[CharList.SelectedIndex];
            DescList.Items.Clear();

            // Get all the child descriptors of a characteristics. Use the cache mode to specify uncached descriptors only 
            // and the new Async functions to get the descriptors of unpaired devices as well. 
            var result = await selectedCharacteristic.GetDescriptorsAsync(BluetoothCacheMode.Uncached);
            if (result.Status != GattCommunicationStatus.Success)
            {
                Console.WriteLine("Descriptor read failure: " + result.Status.ToString());
            }
            else
            {
                DescList.Items.Add(String.Format("Property: {0}", selectedCharacteristic.CharacteristicProperties));
                Console.WriteLine("Property: {0}", selectedCharacteristic.CharacteristicProperties);
            }
        }
    }
}

public class BLEDevice
{
    public ulong DevAddr;
    public string DevName;
    public IList<Guid> ServiceUuids;
}
