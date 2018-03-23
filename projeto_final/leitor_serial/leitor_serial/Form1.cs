using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;

namespace leitor_serial
{
    
    public partial class Form1 : Form
    {
        string RxString;
        int pacote_recebido;
        DateTime tempoInicio, tempoLeitura;

        public Form1()
        {
            InitializeComponent();
        }

        private void atualizaListaCOMs()
        {
            int i;
            bool quantDiferente;//flag para sinalizar que a quantidade de portas mudou

            i = 0;
            quantDiferente = false;

            //se a quantidade de portas mudou
            if (comboBox1.Items.Count == SerialPort.GetPortNames().Length)
            {
                foreach (string s in SerialPort.GetPortNames())
                {
                    if (comboBox1.Items[i++].Equals(s) == false)
                    {
                        quantDiferente = true;
                    }
                }

            }
            else
            {
                quantDiferente = true;
            }

            //Se não foi detectado diferença
            if (quantDiferente == false)
            {
                return;                     //retorna
            }

            //limpa comboBox
            comboBox1.Items.Clear();

            //adiciona todas as COM diponíveis na lista 
            foreach (string s in SerialPort.GetPortNames())
            {
                comboBox1.Items.Add(s);
            }
            //seleciona a primeira posição da lista
            if (comboBox1.Items.Count > 0)
            {
                comboBox1.SelectedIndex = 0;
            }
            else
            {
                comboBox1.Text = " ";
            }
        }

        private void trataDadoRecebido(object sender, EventArgs e)
        {
            //Um dado recebido é composto por um cmd (comando) e um valor associado.
            //dessa forma é possível examinar qual ação tomar sem ter salvo o comando enviado anteriormente

            //textBoxReceber.AppendText(RxString + "\n");
            int cmd = pacote_recebido / 100000;
            int valor = pacote_recebido - cmd * 100000;

            //Console.WriteLine("pct_recebido: " + pacote_recebido);
            //Console.WriteLine(cmd);
            //Console.WriteLine(valor);

            textBoxReceber.AppendText("\n\n == Recebido pacote << \n");
            textBoxReceber.AppendText("cmd: " + pacote_recebido.ToString() + "\n");

            TimeSpan span = this.tempoLeitura - this.tempoInicio;
            int ms = (int)span.TotalSeconds;

            this.dataGridView1.Rows.Add(ms, pacote_recebido);
            tempoLeitura = DateTime.Now;
        }

        private void porta_serial_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            //dados recebidos
            RxString = serialPort1.ReadLine();              //le o dado disponível na serialx
            pacote_recebido = Int32.Parse(RxString);
            this.Invoke(new EventHandler(trataDadoRecebido));   //chama outra thread para escrever o dado no text box

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            atualizaListaCOMs();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            //atualizaListaCOMs();
        }

        private void btnConectar_Click(object sender, EventArgs e)
        {
            if (serialPort1.IsOpen == false)
            {
                try
                {
                    serialPort1.PortName = comboBox1.Items[comboBox1.SelectedIndex].ToString();
                    serialPort1.Open();
                }
                catch
                {
                    return;

                }
                if (serialPort1.IsOpen)
                {
                    btnConectar.Text = "Desconectar";
                    comboBox1.Enabled = false;
                    textBoxReceber.AppendText("\n\n == Conectado ao receptor secundário == \n\n");

                    tempoInicio= DateTime.Now;
                    tempoLeitura = tempoInicio;
                }
            }
            else
            {

                try
                {
                    serialPort1.Close();
                    comboBox1.Enabled = true;
                    btnConectar.Text = "Conectar";
                    textBoxReceber.AppendText("\n\n == Desconectado do receptor == \n\n");
                }
                catch
                {
                    return;
                }

            }
        }
    }
}
