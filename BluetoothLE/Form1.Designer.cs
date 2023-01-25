
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
            this.ServiceBtn = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.ServiceList = new System.Windows.Forms.ListBox();
            this.label3 = new System.Windows.Forms.Label();
            this.CharList = new System.Windows.Forms.ListBox();
            this.label2 = new System.Windows.Forms.Label();
            this.DescList = new System.Windows.Forms.ListBox();
            this.label4 = new System.Windows.Forms.Label();
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
            // ServiceBtn
            // 
            this.ServiceBtn.Location = new System.Drawing.Point(12, 177);
            this.ServiceBtn.Name = "ServiceBtn";
            this.ServiceBtn.Size = new System.Drawing.Size(75, 51);
            this.ServiceBtn.TabIndex = 4;
            this.ServiceBtn.Text = "獲取服務";
            this.ServiceBtn.UseVisualStyleBackColor = true;
            this.ServiceBtn.Click += new System.EventHandler(this.ServiceBtn_Click);
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
            this.ServiceList.Location = new System.Drawing.Point(93, 177);
            this.ServiceList.Name = "ServiceList";
            this.ServiceList.Size = new System.Drawing.Size(400, 79);
            this.ServiceList.TabIndex = 6;
            this.ServiceList.SelectedIndexChanged += new System.EventHandler(this.ServiceList_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(94, 155);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(125, 15);
            this.label3.TabIndex = 7;
            this.label3.Text = "服務 UUID 列表：";
            // 
            // CharList
            // 
            this.CharList.FormattingEnabled = true;
            this.CharList.ItemHeight = 15;
            this.CharList.Location = new System.Drawing.Point(93, 264);
            this.CharList.Name = "CharList";
            this.CharList.Size = new System.Drawing.Size(400, 79);
            this.CharList.TabIndex = 8;
            this.CharList.SelectedIndexChanged += new System.EventHandler(this.CharList_SelectedIndexChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(50, 264);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(37, 15);
            this.label2.TabIndex = 9;
            this.label2.Text = "特徵";
            // 
            // DescList
            // 
            this.DescList.FormattingEnabled = true;
            this.DescList.ItemHeight = 15;
            this.DescList.Location = new System.Drawing.Point(93, 353);
            this.DescList.Name = "DescList";
            this.DescList.Size = new System.Drawing.Size(400, 79);
            this.DescList.TabIndex = 10;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(50, 353);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(37, 15);
            this.label4.TabIndex = 11;
            this.label4.Text = "描述";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(709, 450);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.DescList);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.CharList);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.ServiceList);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.ServiceBtn);
            this.Controls.Add(this.ConnBtn);
            this.Controls.Add(this.DiscoverBtn);
            this.Controls.Add(this.ResultList);
            this.Name = "Form1";
            this.Text = "BluetoothLE controller";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox ResultList;
        private System.Windows.Forms.Button DiscoverBtn;
        private System.Windows.Forms.Button ConnBtn;
        private System.Windows.Forms.Button ServiceBtn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListBox ServiceList;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ListBox CharList;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ListBox DescList;
        private System.Windows.Forms.Label label4;
    }
}

