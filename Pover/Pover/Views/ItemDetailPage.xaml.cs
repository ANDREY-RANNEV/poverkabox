using Pover.ViewModels;
using System.ComponentModel;
using Xamarin.Forms;

namespace Pover.Views
{
    public partial class ItemDetailPage : ContentPage
    {
        public ItemDetailPage()
        {
            InitializeComponent();
            BindingContext = new ItemDetailViewModel();
        }
    }
}