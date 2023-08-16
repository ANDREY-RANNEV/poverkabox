using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Plugin.BLE;
using Plugin.BLE.Abstractions;
using Plugin.BLE.Abstractions.Contracts;
using Plugin.BLE.Abstractions.Exceptions;
using Xamarin.Forms;

namespace Pover
{
    public partial class MainPage : ContentPage
    {
        IAdapter adapter;
        IBluetoothLE bluetoothBLE;
        ObservableCollection<IDevice> list = new ObservableCollection<IDevice>();

        private IDevice device;
        public MainPage()
        {
            InitializeComponent();

            bluetoothBLE = CrossBluetoothLE.Current;
            adapter = CrossBluetoothLE.Current.Adapter;

            list = new ObservableCollection<IDevice>();
            DevicesList.ItemsSource = list;
        }
        private async void searchDevice(object sender, EventArgs e)
        {
            if (bluetoothBLE.State == BluetoothState.Off)
            {
                await DisplayAlert("Atenção", "Bluetooth desabilitado.", "OK");
            }
            else
            {
                list.Clear();

                adapter.ScanTimeout = 60000;
                adapter.ScanMode = ScanMode.Balanced;


                adapter.DeviceDiscovered += (obj, a) =>
                {
                    if (!list.Contains(a.Device))
                        list.Add(a.Device);
                };

                await adapter.StartScanningForDevicesAsync();

            }

        }


        private async void DevicesList_OnItemSelected(object sender, SelectedItemChangedEventArgs e)
        {
            device = DevicesList.SelectedItem as IDevice;

            var result = await DisplayAlert("AVISO", "Deseja se conectar a esse dispositivo?", "Conectar", "Cancelar");

            if (!result)
                return;

            //Stop Scanner
            await adapter.StopScanningForDevicesAsync();

            try
            {
                await adapter.ConnectToDeviceAsync(device);

                await DisplayAlert("Conectado", "Status:" + device.State, "OK");

            }
            catch (DeviceConnectionException ex)
            {
                await DisplayAlert("Erro", ex.Message, "OK");
            }

        }
    }
}
