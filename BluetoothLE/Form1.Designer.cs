
namespace BluetoothLE
{
    partial class Form1
    {
        /// <summary>
        /// 設計工具所需的變數。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清除任何使用中的資源。
        /// </summary>
        /// <param name="disposing">如果應該處置受控資源則為 true，否則為 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 設計工具產生的程式碼

        /// <summary>
        /// 此為設計工具支援所需的方法 - 請勿使用程式碼編輯器修改
        /// 這個方法的內容。
        /// </summary>
        private void InitializeComponent()
        {
            this.ResultList = new System.Windows.Forms.ListBox();
            this.DiscoverBtn = new System.Windows.Forms.Button();
            this.ConnBtn = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.ServiceList = new System.Windows.Forms.ListBox();
            this.label3 = new System.Windows.Forms.Label();
            this.CharList = new System.Windows.Forms.ListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.DescList = new System.Windows.Forms.ListBox();
            this.label4 = new System.Windows.Forms.Label();
            this.NotifyBtn = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.WriteText = new System.Windows.Forms.TextBox();
            this.checkRHex = new System.Windows.Forms.CheckBox();
            this.ReadText = new System.Windows.Forms.TextBox();
            this.WriteBtn = new System.Windows.Forms.Button();
            this.ReadBtn = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label14 = new System.Windows.Forms.Label();
            this.label13 = new System.Windows.Forms.Label();
            this.spdText = new System.Windows.Forms.TextBox();
            this.angleText = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.kdsText = new System.Windows.Forms.TextBox();
            this.kisText = new System.Windows.Forms.TextBox();
            this.kpsText = new System.Windows.Forms.TextBox();
            this.kdaText = new System.Windows.Forms.TextBox();
            this.kiaText = new System.Windows.Forms.TextBox();
            this.kpaText = new System.Windows.Forms.TextBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // ResultList
            // 
            this.ResultList.FormattingEnabled = true;
            this.ResultList.ItemHeight = 15;
            this.ResultList.Location = new System.Drawing.Point(93, 32);
            this.ResultList.Name = "ResultList";
            this.ResultList.Size = new System.Drawing.Size(400, 109);
            this.ResultList.TabIndex = 0;
            this.ResultList.SelectedIndexChanged += new System.EventHandler(this.ResultList_SelectedIndexChanged);
            // 
            // DiscoverBtn
            // 
            this.DiscoverBtn.Location = new System.Drawing.Point(12, 32);
            this.DiscoverBtn.Name = "DiscoverBtn";
            this.DiscoverBtn.Size = new System.Drawing.Size(75, 51);
            this.DiscoverBtn.TabIndex = 1;
            this.DiscoverBtn.Text = "列舉BLE";
            this.DiscoverBtn.UseVisualStyleBackColor = true;
            this.DiscoverBtn.Click += new System.EventHandler(this.DiscoverBtn_Click);
            // 
            // ConnBtn
            // 
            this.ConnBtn.Location = new System.Drawing.Point(12, 90);
            this.ConnBtn.Name = "ConnBtn";
            this.ConnBtn.Size = new System.Drawing.Size(75, 51);
            this.ConnBtn.TabIndex = 3;
            this.ConnBtn.Text = "連線BLE";
            this.ConnBtn.UseVisualStyleBackColor = true;
            this.ConnBtn.Click += new System.EventHandler(this.ConnBtn_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(94, 10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(113, 15);
            this.label1.TabIndex = 5;
            this.label1.Text = "BLE 裝置列表：";
            // 
            // ServiceList
            // 
            this.ServiceList.FormattingEnabled = true;
            this.ServiceList.ItemHeight = 15;
            this.ServiceList.Location = new System.Drawing.Point(93, 154);
            this.ServiceList.Name = "ServiceList";
            this.ServiceList.Size = new System.Drawing.Size(400, 79);
            this.ServiceList.TabIndex = 6;
            this.ServiceList.SelectedIndexChanged += new System.EventHandler(this.ServiceList_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(50, 154);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(37, 15);
            this.label3.TabIndex = 7;
            this.label3.Text = "服務";
            // 
            // CharList
            // 
            this.CharList.FormattingEnabled = true;
            this.CharList.ItemHeight = 15;
            this.CharList.Location = new System.Drawing.Point(93, 241);
            this.CharList.Name = "CharList";
            this.CharList.Size = new System.Drawing.Size(400, 79);
            this.CharList.TabIndex = 8;
            this.CharList.SelectedIndexChanged += new System.EventHandler(this.CharList_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(50, 241);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(37, 15);
            this.label2.TabIndex = 9;
            this.label2.Text = "特徵";
            // 
            // DescList
            // 
            this.DescList.FormattingEnabled = true;
            this.DescList.ItemHeight = 15;
            this.DescList.Location = new System.Drawing.Point(93, 330);
            this.DescList.Name = "DescList";
            this.DescList.Size = new System.Drawing.Size(400, 79);
            this.DescList.TabIndex = 10;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(50, 330);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(37, 15);
            this.label4.TabIndex = 11;
            this.label4.Text = "描述";
            // 
            // NotifyBtn
            // 
            this.NotifyBtn.Enabled = false;
            this.NotifyBtn.Location = new System.Drawing.Point(23, 26);
            this.NotifyBtn.Name = "NotifyBtn";
            this.NotifyBtn.Size = new System.Drawing.Size(55, 38);
            this.NotifyBtn.TabIndex = 12;
            this.NotifyBtn.Text = "通知";
            this.NotifyBtn.UseVisualStyleBackColor = true;
            this.NotifyBtn.Click += new System.EventHandler(this.NotifyBtn_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.WriteText);
            this.groupBox1.Controls.Add(this.checkRHex);
            this.groupBox1.Controls.Add(this.ReadText);
            this.groupBox1.Controls.Add(this.WriteBtn);
            this.groupBox1.Controls.Add(this.ReadBtn);
            this.groupBox1.Controls.Add(this.NotifyBtn);
            this.groupBox1.Location = new System.Drawing.Point(514, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(418, 221);
            this.groupBox1.TabIndex = 13;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "BLE 裝置通訊資料";
            // 
            // WriteText
            // 
            this.WriteText.BackColor = System.Drawing.SystemColors.Control;
            this.WriteText.Location = new System.Drawing.Point(95, 155);
            this.WriteText.Multiline = true;
            this.WriteText.Name = "WriteText";
            this.WriteText.Size = new System.Drawing.Size(300, 51);
            this.WriteText.TabIndex = 17;
            this.WriteText.Text = "0.01, 0.02, 0.03, 0.04, 0.05, 0.06";
            // 
            // checkRHex
            // 
            this.checkRHex.AutoSize = true;
            this.checkRHex.Location = new System.Drawing.Point(95, 45);
            this.checkRHex.Name = "checkRHex";
            this.checkRHex.Size = new System.Drawing.Size(79, 19);
            this.checkRHex.TabIndex = 16;
            this.checkRHex.Text = "Hex data";
            this.checkRHex.UseVisualStyleBackColor = true;
            // 
            // ReadText
            // 
            this.ReadText.BackColor = System.Drawing.SystemColors.GradientInactiveCaption;
            this.ReadText.Location = new System.Drawing.Point(95, 78);
            this.ReadText.Multiline = true;
            this.ReadText.Name = "ReadText";
            this.ReadText.ReadOnly = true;
            this.ReadText.Size = new System.Drawing.Size(300, 51);
            this.ReadText.TabIndex = 15;
            // 
            // WriteBtn
            // 
            this.WriteBtn.Enabled = false;
            this.WriteBtn.Location = new System.Drawing.Point(23, 155);
            this.WriteBtn.Name = "WriteBtn";
            this.WriteBtn.Size = new System.Drawing.Size(55, 38);
            this.WriteBtn.TabIndex = 14;
            this.WriteBtn.Text = "寫";
            this.WriteBtn.UseVisualStyleBackColor = true;
            this.WriteBtn.Click += new System.EventHandler(this.WriteBtn_Click);
            // 
            // ReadBtn
            // 
            this.ReadBtn.Enabled = false;
            this.ReadBtn.Location = new System.Drawing.Point(23, 78);
            this.ReadBtn.Name = "ReadBtn";
            this.ReadBtn.Size = new System.Drawing.Size(55, 38);
            this.ReadBtn.TabIndex = 13;
            this.ReadBtn.Text = "讀";
            this.ReadBtn.UseVisualStyleBackColor = true;
            this.ReadBtn.Click += new System.EventHandler(this.ReadBtn_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label14);
            this.groupBox2.Controls.Add(this.label13);
            this.groupBox2.Controls.Add(this.spdText);
            this.groupBox2.Controls.Add(this.angleText);
            this.groupBox2.Controls.Add(this.label12);
            this.groupBox2.Controls.Add(this.label11);
            this.groupBox2.Controls.Add(this.label10);
            this.groupBox2.Controls.Add(this.label9);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label6);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.kdsText);
            this.groupBox2.Controls.Add(this.kisText);
            this.groupBox2.Controls.Add(this.kpsText);
            this.groupBox2.Controls.Add(this.kdaText);
            this.groupBox2.Controls.Add(this.kiaText);
            this.groupBox2.Controls.Add(this.kpaText);
            this.groupBox2.Location = new System.Drawing.Point(514, 241);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(418, 169);
            this.groupBox2.TabIndex = 14;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "平衡車參數";
            // 
            // label14
            // 
            this.label14.AutoSize = true;
            this.label14.Location = new System.Drawing.Point(243, 131);
            this.label14.Name = "label14";
            this.label14.Size = new System.Drawing.Size(82, 15);
            this.label14.TabIndex = 17;
            this.label14.Text = "齒輪轉速：";
            // 
            // label13
            // 
            this.label13.AutoSize = true;
            this.label13.Location = new System.Drawing.Point(20, 131);
            this.label13.Name = "label13";
            this.label13.Size = new System.Drawing.Size(82, 15);
            this.label13.TabIndex = 16;
            this.label13.Text = "偏移角度：";
            // 
            // spdText
            // 
            this.spdText.Location = new System.Drawing.Point(331, 128);
            this.spdText.Name = "spdText";
            this.spdText.ReadOnly = true;
            this.spdText.Size = new System.Drawing.Size(64, 25);
            this.spdText.TabIndex = 15;
            this.spdText.Text = "0.0";
            this.spdText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // angleText
            // 
            this.angleText.Location = new System.Drawing.Point(106, 128);
            this.angleText.Name = "angleText";
            this.angleText.ReadOnly = true;
            this.angleText.Size = new System.Drawing.Size(64, 25);
            this.angleText.TabIndex = 14;
            this.angleText.Text = "0.0";
            this.angleText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label12
            // 
            this.label12.AutoSize = true;
            this.label12.Location = new System.Drawing.Point(20, 79);
            this.label12.Name = "label12";
            this.label12.Size = new System.Drawing.Size(52, 15);
            this.label12.TabIndex = 13;
            this.label12.Text = "轉速：";
            // 
            // label11
            // 
            this.label11.AutoSize = true;
            this.label11.Location = new System.Drawing.Point(20, 35);
            this.label11.Name = "label11";
            this.label11.Size = new System.Drawing.Size(52, 15);
            this.label11.TabIndex = 12;
            this.label11.Text = "角度：";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Location = new System.Drawing.Point(307, 79);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(21, 15);
            this.label10.TabIndex = 11;
            this.label10.Text = "kd";
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(307, 35);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(21, 15);
            this.label9.TabIndex = 10;
            this.label9.Text = "kd";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(193, 79);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(18, 15);
            this.label8.TabIndex = 9;
            this.label8.Text = "ki";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(193, 35);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(18, 15);
            this.label7.TabIndex = 8;
            this.label7.Text = "ki";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(79, 79);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(21, 15);
            this.label6.TabIndex = 7;
            this.label6.Text = "kp";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(79, 35);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(21, 15);
            this.label5.TabIndex = 6;
            this.label5.Text = "kp";
            // 
            // kdsText
            // 
            this.kdsText.Location = new System.Drawing.Point(331, 76);
            this.kdsText.Name = "kdsText";
            this.kdsText.ReadOnly = true;
            this.kdsText.Size = new System.Drawing.Size(64, 25);
            this.kdsText.TabIndex = 5;
            this.kdsText.Text = "0.0";
            this.kdsText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // kisText
            // 
            this.kisText.Location = new System.Drawing.Point(217, 76);
            this.kisText.Name = "kisText";
            this.kisText.ReadOnly = true;
            this.kisText.Size = new System.Drawing.Size(64, 25);
            this.kisText.TabIndex = 4;
            this.kisText.Text = "0.0";
            this.kisText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // kpsText
            // 
            this.kpsText.Location = new System.Drawing.Point(106, 76);
            this.kpsText.Name = "kpsText";
            this.kpsText.ReadOnly = true;
            this.kpsText.Size = new System.Drawing.Size(64, 25);
            this.kpsText.TabIndex = 3;
            this.kpsText.Text = "0.0";
            this.kpsText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // kdaText
            // 
            this.kdaText.Location = new System.Drawing.Point(331, 32);
            this.kdaText.Name = "kdaText";
            this.kdaText.ReadOnly = true;
            this.kdaText.Size = new System.Drawing.Size(64, 25);
            this.kdaText.TabIndex = 2;
            this.kdaText.Text = "0.0";
            this.kdaText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // kiaText
            // 
            this.kiaText.Location = new System.Drawing.Point(217, 32);
            this.kiaText.Name = "kiaText";
            this.kiaText.ReadOnly = true;
            this.kiaText.Size = new System.Drawing.Size(64, 25);
            this.kiaText.TabIndex = 1;
            this.kiaText.Text = "0.0";
            this.kiaText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // kpaText
            // 
            this.kpaText.Location = new System.Drawing.Point(106, 32);
            this.kpaText.Name = "kpaText";
            this.kpaText.ReadOnly = true;
            this.kpaText.Size = new System.Drawing.Size(64, 25);
            this.kpaText.TabIndex = 0;
            this.kpaText.Text = "0.0";
            this.kpaText.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(946, 422);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.DescList);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.CharList);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.ServiceList);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.ConnBtn);
            this.Controls.Add(this.DiscoverBtn);
            this.Controls.Add(this.ResultList);
            this.Name = "Form1";
            this.Text = "BluetoothLE controller";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox ResultList;
        private System.Windows.Forms.Button DiscoverBtn;
        private System.Windows.Forms.Button ConnBtn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListBox ServiceList;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ListBox CharList;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListBox DescList;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button NotifyBtn;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button ReadBtn;
        private System.Windows.Forms.Button WriteBtn;
        private System.Windows.Forms.TextBox ReadText;
        private System.Windows.Forms.CheckBox checkRHex;
        private System.Windows.Forms.TextBox WriteText;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox kdsText;
        private System.Windows.Forms.TextBox kisText;
        private System.Windows.Forms.TextBox kpsText;
        private System.Windows.Forms.TextBox kdaText;
        private System.Windows.Forms.TextBox kiaText;
        private System.Windows.Forms.TextBox kpaText;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label12;
        private System.Windows.Forms.Label label11;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox spdText;
        private System.Windows.Forms.TextBox angleText;
        private System.Windows.Forms.Label label14;
        private System.Windows.Forms.Label label13;
    }
}

