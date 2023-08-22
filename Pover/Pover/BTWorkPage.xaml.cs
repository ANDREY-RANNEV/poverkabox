using Plugin.BluetoothClassic.Abstractions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace Pover
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class BTWorkPage : ContentPage
    {
        IBluetoothConnection Connection;
        BluetoothDeviceModel BluetoothDevice;
        IBluetoothAdapter BluetoothAdapter;

        public BTWorkPage()
        {
            InitializeComponent();
            BluetoothAdapter = DependencyService.Resolve<IBluetoothAdapter>();
            BluetoothDevice = (BluetoothDeviceModel)BindingContext;
            if (BluetoothDevice != null)
            {
                Connection = BluetoothAdapter.CreateConnection(BluetoothDevice);
                Connection.ConnectAsync().Wait();
            }
        }

        private async void connectbt_Clicked(object sender, EventArgs e)
        {
            var device=(BluetoothDeviceModel)BindingContext;
            if (device != null)
            {
                var adapter = DependencyService.Resolve<IBluetoothAdapter>();
                using (var connection=adapter.CreateConnection(device))
                {
                    if(await connection.RetryConnectAsync(retriesCount:2))
                    {
                        byte[] buffer = Encoding.ASCII.GetBytes("{\"start\":1}");
                        if(!(await connection.RetryTransmitAsync(buffer,0,buffer.Length)))
                        {
                            DisplayAlert("error", "cannot", "Close");
                        }
                    }
                    else
                    {
                        DisplayAlert("error", "cannot", "Close");
                    }
                }
            }

        }
    }
}