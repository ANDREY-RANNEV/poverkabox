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
    public partial class selectDevicePage : ContentPage
    {
        public selectDevicePage()
        {
            InitializeComponent();
            fillDevices();
        }

        private void fillDevices()
        {
            var adapter = DependencyService.Resolve<IBluetoothAdapter>();
            BoundedDevices.ItemsSource = adapter.BondedDevices;

        }

        private  async void BoundedDevices_ItemSelected(object sender, SelectedItemChangedEventArgs e)
        {
            var device = (BluetoothDeviceModel)e.SelectedItem;
            if (device != null)
            {
                await Navigation.PushAsync(new BTWorkPage() { BindingContext=device});
            }

            BoundedDevices.SelectedItem = null;
        }
    }
}