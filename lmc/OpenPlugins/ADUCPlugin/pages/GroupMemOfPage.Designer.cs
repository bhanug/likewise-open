/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

using Likewise.LMC.Utilities;

namespace Likewise.LMC.Plugins.ADUCPlugin
{
    partial class GroupMemOfPage
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GroupMemOfPage));
            this.MemOflabel = new System.Windows.Forms.Label();
            this.MemoflistView = new Likewise.LMC.ServerControl.LWListView();
            this.Namecolumn = new System.Windows.Forms.ColumnHeader();
            this.ACFolerColumn = new System.Windows.Forms.ColumnHeader();
            this.LargeImageList = new System.Windows.Forms.ImageList(this.components);
            this.smallImageList = new System.Windows.Forms.ImageList(this.components);
            this.Addbutton = new System.Windows.Forms.Button();
            this.RemoveButton = new System.Windows.Forms.Button();
            this.DomainUserlabel = new System.Windows.Forms.Label();
            this.setGrouplabel = new System.Windows.Forms.Label();
            this.pnlData.SuspendLayout();
            this.SuspendLayout();
            //
            // pnlData
            //
            this.pnlData.Controls.Add(this.setGrouplabel);
            this.pnlData.Controls.Add(this.DomainUserlabel);
            this.pnlData.Controls.Add(this.RemoveButton);
            this.pnlData.Controls.Add(this.Addbutton);
            this.pnlData.Controls.Add(this.MemoflistView);
            this.pnlData.Controls.Add(this.MemOflabel);
            this.pnlData.Size = new System.Drawing.Size(371, 426);
            //
            // MemOflabel
            //
            this.MemOflabel.AutoSize = true;
            this.MemOflabel.Location = new System.Drawing.Point(7, 18);
            this.MemOflabel.Name = "MemOflabel";
            this.MemOflabel.Size = new System.Drawing.Size(60, 13);
            this.MemOflabel.TabIndex = 0;
            this.MemOflabel.Text = "Member of:";
            //
            // MemoflistView
            //
            this.MemoflistView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Namecolumn,
            this.ACFolerColumn});
            this.MemoflistView.FullRowSelect = true;
            this.MemoflistView.HideSelection = false;
            this.MemoflistView.LargeImageList = this.LargeImageList;
            this.MemoflistView.Location = new System.Drawing.Point(9, 37);
            this.MemoflistView.Name = "MemoflistView";
            this.MemoflistView.ShowGroups = false;
            this.MemoflistView.Size = new System.Drawing.Size(352, 251);
            this.MemoflistView.SmallImageList = this.smallImageList;
            this.MemoflistView.TabIndex = 0;
            this.MemoflistView.UseCompatibleStateImageBehavior = false;
            this.MemoflistView.View = System.Windows.Forms.View.Details;
            //
            // Namecolumn
            //
            this.Namecolumn.Text = "Name";
            this.Namecolumn.Width = 158;
            //
            // ACFolerColumn
            //
            this.ACFolerColumn.Text = "Active Directory Folder";
            this.ACFolerColumn.Width = 274;
            //
            // LargeImageList
            //
            this.LargeImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("LargeImageList.ImageStream")));
            this.LargeImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.LargeImageList.Images.SetKeyName(0, "computer_48.bmp");
            this.LargeImageList.Images.SetKeyName(1, "Group_48.bmp");
            this.LargeImageList.Images.SetKeyName(2, "user_48.bmp");
            this.LargeImageList.Images.SetKeyName(3, "Folder.ico");
            this.LargeImageList.Images.SetKeyName(4, "aduc_48.bmp");
            this.LargeImageList.Images.SetKeyName(5, "Admin.ico");
            this.LargeImageList.Images.SetKeyName(6, "ADUC.ico");
            this.LargeImageList.Images.SetKeyName(7, "DisabledUser.ico");
            this.LargeImageList.Images.SetKeyName(8, "DisabledComp.ico");
            //
            // smallImageList
            //
            this.smallImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("smallImageList.ImageStream")));
            this.smallImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.smallImageList.Images.SetKeyName(0, "computer_16.bmp");
            this.smallImageList.Images.SetKeyName(1, "group_16.bmp");
            this.smallImageList.Images.SetKeyName(2, "user_16.bmp");
            this.smallImageList.Images.SetKeyName(3, "Folder.ico");
            this.smallImageList.Images.SetKeyName(4, "aduc_16.bmp");
            this.smallImageList.Images.SetKeyName(5, "Admin.ico");
            this.smallImageList.Images.SetKeyName(6, "ADUC.ico");
            this.smallImageList.Images.SetKeyName(7, "DisabledUser.ico");
            this.smallImageList.Images.SetKeyName(8, "DisabledComp.ico");
            //
            // Addbutton
            //
            this.Addbutton.Location = new System.Drawing.Point(10, 391);
            this.Addbutton.Name = "Addbutton";
            this.Addbutton.Size = new System.Drawing.Size(75, 23);
            this.Addbutton.TabIndex = 2;
            this.Addbutton.Text = "A&dd...";
            this.Addbutton.UseVisualStyleBackColor = true;
            //
            // RemoveButton
            //
            this.RemoveButton.Enabled = false;
            this.RemoveButton.Location = new System.Drawing.Point(99, 391);
            this.RemoveButton.Name = "RemoveButton";
            this.RemoveButton.Size = new System.Drawing.Size(75, 23);
            this.RemoveButton.TabIndex = 3;
            this.RemoveButton.Text = "&Remove";
            this.RemoveButton.UseVisualStyleBackColor = true;
            //
            // DomainUserlabel
            //
            this.DomainUserlabel.AutoSize = true;
            this.DomainUserlabel.Location = new System.Drawing.Point(123, 291);
            this.DomainUserlabel.Name = "DomainUserlabel";
            this.DomainUserlabel.Size = new System.Drawing.Size(0, 13);
            this.DomainUserlabel.TabIndex = 6;
            //
            // setGrouplabel
            //
            this.setGrouplabel.Location = new System.Drawing.Point(14, 303);
            this.setGrouplabel.Name = "setGrouplabel";
            this.setGrouplabel.Size = new System.Drawing.Size(347, 74);
            this.setGrouplabel.TabIndex = 8;
            //
            // GroupMemOfPage
            //
            this.Name = "GroupMemOfPage";
            this.Size = new System.Drawing.Size(371, 426);
            this.pnlData.ResumeLayout(false);
            this.pnlData.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Label MemOflabel;
        private Likewise.LMC.ServerControl.LWListView MemoflistView;
        private System.Windows.Forms.ColumnHeader Namecolumn;
        private System.Windows.Forms.ColumnHeader ACFolerColumn;
        private System.Windows.Forms.Button RemoveButton;
        private System.Windows.Forms.Button Addbutton;
        private System.Windows.Forms.Label DomainUserlabel;
        private System.Windows.Forms.Label setGrouplabel;
        private System.Windows.Forms.ImageList smallImageList;
        private System.Windows.Forms.ImageList LargeImageList;
    }
}