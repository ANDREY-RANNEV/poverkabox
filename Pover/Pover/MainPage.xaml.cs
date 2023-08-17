using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Shiny;
using Xamarin.Essentials;
using Xamarin.Forms;

namespace Pover
{
    public partial class MainPage : ContentPage
    {
        IAdapter BluetoothAdapter;
        public MainPage()
        {
            InitializeComponent();
            BluetoothAdapter.sta();
        }

    }
}
