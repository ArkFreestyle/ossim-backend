<?php
/**
*
* License:
*
* Copyright (c) 2003-2006 ossim.net
* Copyright (c) 2007-2013 AlienVault
* All rights reserved.
*
* This package is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; version 2 dated June, 1991.
* You may not use, modify or distribute this program under any other version
* of the GNU General Public License.
*
* This package is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this package; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
* MA  02110-1301  USA
*
*
* On Debian GNU/Linux systems, the complete text of the GNU General
* Public License can be found in `/usr/share/common-licenses/GPL-2'.
*
* Otherwise you can read it here: http://www.gnu.org/licenses/gpl-2.0.txt
*
*/


//
// $Id: reshtml.php,v 1.12 2010/04/26 16:08:21 josedejoses Exp $
//

/***********************************************************/
/*                 Inprotect                               */
/* --------------------------------------------------------*/
/* Copyright (C) 2006 Inprotect                            */
/*                                                         */
/* This program is free software; you can redistribute it  */
/* and/or modify it under the terms of version 2 of the    */
/* GNU General Public License as published by the Free     */
/* Software Foundation.                                    */
/* This program is distributed in the hope that it will be */
/* useful, but WITHOUT ANY WARRANTY; without even the      */
/* implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/* PARTICULAR PURPOSE. See the GNU General Public License  */
/* for more details.                                       */
/*                                                         */
/* You should have received a copy of the GNU General      */
/* Public License along with this program; if not, write   */
/* to the Free Software Foundation, Inc., 59 Temple Place, */
/* Suite 330, Boston, MA 02111-1307 USA                    */
/*                                                         */
/* Contact Information:                                    */
/* inprotect-devel@lists.sourceforge.net                   */
/* http://inprotect.sourceforge.net/                       */
/***********************************************************/
/* See the README.txt and/or help files for more           */
/* information on how to use & config.                     */
/* See the LICENSE.txt file for more information on the    */
/* License this software is distributed under.             */
/*                                                         */
/* This program is intended for use in an authorized       */
/* manner only, and the author can not be held liable for  */
/* anything done with this program, code, or items         */
/* discovered with this program's use.                     */
/***********************************************************/

ini_set("max_execution_time","720");

require_once 'av_init.php';
require_once 'config.php';
require_once 'functions.inc';
require_once 'ossim_sql.inc';

Session::logcheck('environment-menu', 'EventsVulnerabilities');
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
    <head>
        <title><?php echo _("Vulnerabilities - HTML Report");?></title>
        <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1"/>
        <meta http-equiv="Pragma" content="no-cache">
        <link rel="stylesheet" type="text/css" href="../style/av_common.css?t=<?php echo Util::get_css_id() ?>"/>
        <link rel="stylesheet" type="text/css" href="/ossim/style/tipTip.css"/>
        <link rel="stylesheet" type="text/css" href="/ossim/style/vulnmeter/html_report.css"/>
        <script type="text/javascript" src="../js/jquery.min.js"></script>
        <script type="text/javascript" src="../js/jquery.simpletip.js"></script>
        <script type="text/javascript" src="../js/greybox.js"></script>
        <script type="text/javascript" src="../js/utils.js"></script>
        <script type="text/javascript" src="/ossim/js/jquery.tipTip-ajax.js"></script>
        <?php require '../host_report_menu.php'?>
    </head>
<body>
<?php

$getParams  = array("ctx", "key", "ipl", "treport", "disp", "op", "output", "scantime", "scansubmit", "scantype", "reporttype", "key", "sortby", "allres", "fp","nfp", "wh", "bg", "filterip", "critical", "increment", "pag", "chks" );
$postParams = array("treport", "disp", "op", "output", "scantime", "scansubmit", "scantype", "fp","nfp", "filterip", "critical", "increment", "chks" );

$dbconn->close();
$dbconn = $db->connect();

switch ($_SERVER['REQUEST_METHOD'])
{
    case "GET" :
       foreach($getParams as $gp)
       {
          if (isset($_GET[$gp]))
          {
             $$gp=Util::htmlentities(escape_sql(trim($_GET[$gp]), $dbconn));
          }
          else
          {
             $$gp = '';
          }
       }
    break;

    case "POST" :

       foreach($postParams as $pp)
       {
          if (isset($_POST[$pp]))
          {
             $$pp=Util::htmlentities(escape_sql(trim($_POST[$pp]), $dbconn));
          }
          else
          {
             $$pp = '';
          }
       }

    break;
}

$dbconn->SetFetchMode(ADODB_FETCH_BOTH);

if ($pag == '' || $pag<1)
{
    $pag = 1;
}

list($arruser, $user) = Vulnerabilities::get_users_and_entities_filter($dbconn);

ossim_valid($treport, OSS_ALPHA, OSS_NULLABLE, 'illegal:' . _("Latest Report"));
if (ossim_error())
{
    ossim_error(_("Invalid Parameter treport"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($ipl, OSS_NULLABLE, OSS_IP_ADDRCIDR, 'illegal:' . _("IP latest"));
if (ossim_error() && $ipl!="all")
{
    ossim_error(_("Invalid Parameter ipl"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($key, OSS_NULLABLE, OSS_DIGIT, 'illegal:' . _("Key"));
if (ossim_error())
{
    ossim_error(_("Invalid Parameter Key"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($ctx, OSS_NULLABLE, OSS_HEX, 'illegal:' . _("Context"));
if (ossim_error())
{
    ossim_error(_("Invalid Parameter Context"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($disp, OSS_ALPHA, OSS_NULLABLE, 'illegal:' . _("Report Type"));
if (ossim_error())
{
    ossim_error(_("Invalid Report Type"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($output, OSS_ALPHA, OSS_NULLABLE, 'illegal:' . _("Output Type"));
if (ossim_error())
{
    ossim_error(_("Invalid Output Type"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($scantime, OSS_DIGIT, OSS_NULLABLE, 'illegal:' . _("Scantime"));
if (ossim_error())
{
    ossim_error(_("Invalid Scantime"));
    exit();
}
ossim_set_error(FALSE);

ossim_valid($scantype, OSS_ALPHA, 'illegal:' . _("Scan Type"));
if (ossim_error())
{
    ossim_error(_("Invalid Scan Type"));
    exit();
}

ossim_valid($chks, "t", "f", OSS_NULLABLE, 'illegal:' . _("Chks"));
if (ossim_error())
{
    ossim_error(_("Invalid Chks"));
    exit();
}

ossim_valid($fp, OSS_NULLABLE, OSS_DIGIT, 'illegal:' . _("False positive"));
if (ossim_error())
{
    die(ossim_error());
}

ossim_valid($nfp, OSS_NULLABLE, OSS_DIGIT, 'illegal:' . _("No False positive"));
if (ossim_error())
{
    die(ossim_error());
}
?>

<script type="text/javascript">
   // GrayBox
    $(document).ready(function(){
        GB_TYPE = 'w';
        $("a.greybox").click(function(){
            var t = this.title || $(this).text() || this.href;
            GB_show(t,this.href,550,'90%');
            return false;
        });
        <?php
        if (isset($chks))
        {
            // levels "Critical" => "1", "High" => "2", "Medium" => "3", "Low" => "6", "Info" => "7"
            if (substr($chks,0,1)=="f") echo "$('#checkboxS').prop('checked',false); $('.risk1').hide();";
            if (substr($chks,1,1)=="f") echo "$('#checkboxH').prop('checked',false); $('.risk2').hide();";
            if (substr($chks,2,1)=="f") echo "$('#checkboxM').prop('checked',false); $('.risk3').hide();";
            if (substr($chks,3,1)=="f") echo "$('#checkboxL').prop('checked',false); $('.risk6').hide();";
            if (substr($chks,4,1)=="f") echo "$('#checkboxI').prop('checked',false); $('.risk7').hide();";
        }
        ?>

        // show/hide hosts
        $('.hostip').map(function(idx, element){
        	var vall = false;
        	$('tr.trsk',element).each(function(){
                //$(this).log($(this).css('display'))
                if ($(this).css('display')!='none')
                {
                    vall = true;
                }
        	});

            if (!vall)
            {
                $(element).hide();
            }
            else
            {
                $(element).show();
            }
        });

        $('a.anchor_link').on('click', anchor_link);
    });

    function GB_onhide(url)
	{
		if (url.match(/newincident/))
		{
    		document.location.href="../incidents/index.php?m_opt=analysis&sm_opt=tickets&h_opt=tickets"
		}
	}

    function postload()
    {
        $('.scriptinfo').tipTip({
           defaultPosition: "top",
           delay_load: 100,
           maxWidth: "auto",
           edgeOffset: 3,
           keepAlive:true,
           content: function (e) {
              var id = $(this).attr('lid');

               $.ajax({
                   type: 'GET',
                   data: 'id='+id,
                   url: 'lookup.php',
                   success: function (response) {
                       e.content.html(response); // the var e is the callback function data (see above)
                   }
               });
               return '<?php echo _("Searching")."..."?>'; // We temporary show a Please wait text until the ajax success callback is called.
            }
         });
        $(".checkinfo").simpletip({
            position: 'top',
            baseClass: 'tooltip',
            onBeforeShow: function() {
                this.update('<?=_("Click to enable/disable risk level view")?>');
            }
        });
    }

    function showFalsePositives()
    {
        if ($('#checkboxFP').attr('checked'))
        {
            $('.fp').show();
        }
        else
        {
            $('.fp').hide();
        }
    }

    function toggle_vulns (type)
    {
        if(type=="checkboxS")
        {
            if ($('#checkboxS').attr('checked'))
            {
                $('.risk1').show();
            }
            else
            {
                $('.risk1').hide();
            }
        }
        else if(type=="checkboxH")
        {
            if ($('#checkboxH').attr('checked'))
            {
                $('.risk2').show();
            }
            else
            {
                $('.risk2').hide();
            }
        }
        else if(type=="checkboxM")
        {
            if ($('#checkboxM').attr('checked'))
            {
                $('.risk3').show();
            }
            else
            {
                $('.risk3').hide();
            }
        }
        else if(type=="checkboxL")
        {
            if ($('#checkboxL').attr('checked'))
            {
                $('.risk6').show();
            }
            else
            {
                $('.risk6').hide();
            }
        }
        else if(type=="checkboxI")
        {
            if ($('#checkboxI').attr('checked'))
            {
                $('.risk7').show();
            }
            else
            {
                $('.risk7').hide();
            }
        }

        // checking false positives
        if ($('#checkboxFP').attr('checked'))
        {
            $('.fp').show();
        }
        else
        {
            $('.fp').hide();
        }

        // show/hide hosts
        $('.hostip').map(function(idx, element)
        {
            var vall = false;
            $('tr.trsk',element).each(function(){
                //$(this).log($(this).css('display'))
            	if ($(this).css('display')!='none')
            	{
            	   vall = true;
            	}
            });

            if (!vall)
            {
                $(element).hide();
            }
            else
            {
                $(element).show();
            }
        });
    }

    function jumptopage(url)
    {
        var c1 = $('#checkboxS').is(':checked') ? "t" : "f";
        var c2 = $('#checkboxH').is(':checked') ? "t" : "f";
        var c3 = $('#checkboxM').is(':checked') ? "t" : "f";
        var c4 = $('#checkboxL').is(':checked') ? "t" : "f";
        var c5 = $('#checkboxI').is(':checked') ? "t" : "f";

        document.location.href = url + '&chks=' + c1 + c2 + c3 + c4 + c5;
    }
</script>
<?php

$fp_perms_where = (Session::get_ctx_where() != "") ? " AND vnr.ctx in (".Session::get_ctx_where().")" : "";

if($nfp != '')
{
    $dbconn->execute("UPDATE vuln_nessus_latest_results SET falsepositive='N' WHERE result_id=$nfp".
        ((!empty($arruser))? " AND username in ($user)":""));

    $query = "select vnr.result_id, vnr.hostIP, HEX(vnr.ctx) as ctx from vuln_nessus_results vnr, vuln_nessus_latest_results vnlr
              where
              vnlr.service=vnr.service
              and vnlr.risk=vnr.risk
              and vnlr.scriptid=vnr.scriptid
              and vnlr.hostIP=vnr.hostIP
              and vnlr.ctx=vnr.ctx
              and vnlr.result_id=$nfp".$fp_perms_where.
              ((!empty($arruser))? " AND vnr.username in ($user) ":"");

    $resultfp = $dbconn->execute($query);

    while($resultfp->fields)
    {
        $result_id = $resultfp->fields['result_id'];
        $host_ip   = $resultfp->fields['hostIP'];
        $host_ctx  = $resultfp->fields['ctx'];

        if(!empty($result_id) && Session::hostAllowed_by_ip_ctx($dbconn, $host_ip, $host_ctx))
        {
            $dbconn->execute("UPDATE vuln_nessus_results SET falsepositive='N' WHERE result_id ='$result_id'");
        }

        $resultfp->MoveNext();
    }
}

if($fp != '')
{
    $dbconn->execute("UPDATE vuln_nessus_latest_results SET falsepositive='Y' WHERE result_id=$fp".
        ((!empty($arruser))? " AND username in ($user)":""));

    $query = "select vnr.result_id, vnr.hostIP, HEX(vnr.ctx) as ctx from vuln_nessus_results vnr, vuln_nessus_latest_results vnlr
              where
              vnlr.service=vnr.service
              and vnlr.risk=vnr.risk
              and vnlr.scriptid=vnr.scriptid
              and vnlr.hostIP=vnr.hostIP
              and vnlr.ctx=vnr.ctx
              and vnlr.result_id=$fp".$fp_perms_where.
              ((!empty($arruser))? " AND vnr.username in ($user) ":"");

    $resultfp = $dbconn->execute($query);

    while($resultfp->fields)
    {
        $result_id = $resultfp->fields['result_id'];
        $host_ip   = $resultfp->fields['hostIP'];
        $host_ctx  = $resultfp->fields['ctx'];

        if(!empty($result_id) && Session::hostAllowed_by_ip_ctx($dbconn, $host_ip, $host_ctx))
        {
            $dbconn->execute("UPDATE vuln_nessus_results SET falsepositive='Y' WHERE result_id ='$result_id'");
        }

        $resultfp->MoveNext();
    }
}


function generate_results($output)
{
    global $user, $border, $report_id, $sid, $scantime, $scansubmit, $scantype, $fp, $nfp, $output, $filterip,
    $query_risk, $dbconn, $treport, $ipl, $key;

    echo vulnbreakdown();
    echo hostsummary();
    echo origdetails();
}


function vulnbreakdown()
{
    //GENERATE CHART
    global $user, $border, $report_id, $scantime, $scantype, $fp, $nfp, $output, $filterip, $query_risk, $dbconn;
    global $treport, $sid, $ipl, $ctx, $key;

    $htmlchart  = '';
    $query_host = '';

    if ($filterip)
    {
        $query_host = " AND hostip='$filterip'";
    }

    $perms_where = (Session::get_ctx_where() != "") ? " AND res.ctx in (".Session::get_ctx_where().")" : "";

    $dbconn->SetFetchMode(ADODB_FETCH_BOTH);

    if ($ipl=="all")
    {
        $query ="select hostIP, HEX(ctx) AS ctx, count(*) as count, risk
                    from (select distinct res.port, res.protocol, res.app, res.scriptid, res.risk, res.msg, res.hostIP, res.ctx
                    from vuln_nessus_latest_results res where falsepositive='N' $perms_where)
                    as t group by risk, hostIP, ctx";
    }
    else if (!empty($ipl) && !empty($ctx))
    {
        $query = "select hostIP, HEX(ctx) AS ctx, count(*) as count,risk
        from (select distinct res.port, res.protocol, res.app, res.scriptid, res.risk, res.msg, res.hostIP, res.ctx
        from vuln_nessus_latest_results res where falsepositive='N' and hostIP='$ipl' and ctx=UNHEX('$ctx') $perms_where) as t group by risk, hostIP, ctx";
    }
    else if(!empty($scantime) && !empty($key))
    {
        $query = "select hostIP, HEX(ctx) as ctx, count(*) as count,risk
        from (select distinct res.port, res.protocol, res.app, res.scriptid, res.risk, res.msg, res.hostIP, res.ctx
        from vuln_nessus_latest_results res, vuln_nessus_latest_reports rep
        where res.falsepositive='N'
        and res.scantime='$scantime'
        and res.hostIP=rep.hostIP
        and res.ctx=rep.ctx
        and res.username=rep.username
        and res.sid=rep.sid
        and rep.report_key='$key' $perms_where) as t group by risk, hostIP, ctx";
    }
    //echo $query;

    $result=$dbconn->Execute($query);

    $prevrisk=0;
    $chartimg="./graph1.php?graph=1";

   while ($result->fields)
   {
        $hostip    = $result->fields['hostIP'];
        $hostctx   = $result->fields['ctx'];
        $riskcount = $result->fields['count'];
        $risk      = $result->fields['risk'];

        if(Session::hostAllowed_by_ip_ctx($dbconn, $hostip, $hostctx))
        {
            for ($i=0;$i<$risk-$prevrisk-1;$i++)
            {
                $missedrisk=$prevrisk+$i+1;
                $chartimg.="&amp;risk$missedrisk=0";
            }

            $prevrisk = $risk;

            if ($arr_vulns[$risk] == '')
            {
                $arr_vulns[$risk] = $riskcount;
            }
            else
            {
                $arr_vulns[$risk] += $riskcount;
            }
        }

        $result->MoveNext();
    }

    foreach ($arr_vulns as $avk => $avv)
    {
        $chartimg.="&amp;risk$avk=$avv";
    }

    if (intval($prevrisk)!=7)
    {
        for($i=$prevrisk+1;$i<=7;$i++)
        {
            $chartimg.="&amp;risk$i=0";
        }
    }

    // print out the pie chart
    if($prevrisk!=0)
    {
        $htmlchart .= "<img id='c_chart_vf' alt='Chart' src='$chartimg'>";
    }
    else
    {
        $htmlchart = "<div id='c_nvf'>"._("No vulnerability data")."</div>";
    }

    return $htmlchart;
}


function hostsummary()
{
    global $user, $border, $report_id, $scantime, $scantype, $fp, $nfp, $output, $filterip, $query_risk, $dbconn;
    global $treport, $sid, $ipl, $ips_in_range, $pag, $ctx, $key;

    $htmldetails = '';
    $query_host  = '';

    if ($filterip)
    {
        $query_host = " AND hostip='$filterip'";
    }

    $htmldetails .= "<table id='c_host_summary'><tr><td class=\"headerpr_no_bborder\" style=\"border:0;\"><b>"._("Summary of Scanned Hosts")."</b></td></tr></table>
                     <table id='c_host_stats' summary=\""._("Summary of scanned hosts")."\">";
    $htmldetails .= "<form>";

    $htmldetails .= "<tr><th width=\"128\"><b>"._("Host")."&nbsp;&nbsp;</b></th>
         <th width=\"128\"><b>"._("Hostname")."&nbsp;&nbsp;</b></th>
         <td width=\"128\" style='background-color:#FFCDFF;border-radius: 3px; -moz-border-radius: 3px; -webkit-border-radius: 3px;border: 1px solid #C835ED;'>
            <table width=\"100%\" class=\"noborder\" style=\"background:transparent\">
                <tr>
                    <td width=\"80%\" class=\"nobborder\" style=\"text-align:center;\">
                    <b>"._("Critical")."&nbsp;&nbsp;</b>
                    </td>
                    <td class=\"checkinfo nobborder\" width=\"20%\">
                    <input id=\"checkboxS\" type=\"checkbox\" onclick=\"toggle_vulns('checkboxS')\" checked>
                    </td>
                </tr>
            </table>
         </td>
         <td width=\"128\" style='background-color:#FFDBDB;border-radius: 3px; -moz-border-radius: 3px; -webkit-border-radius: 3px;border: 1px solid #FF0000;'>
            <table width=\"100%\" class=\"noborder\" style=\"background:transparent\">
                <tr>
                    <td width=\"80%\" class=\"nobborder\" style=\"text-align:center;\">
                    <b>"._("High")."&nbsp;&nbsp;</b>
                    </td>
                    <td class=\"checkinfo nobborder\" width=\"20%\">
                    <input id=\"checkboxH\" type=\"checkbox\" onclick=\"toggle_vulns('checkboxH')\" checked>
                    </td>
                </tr>
            </table>
        </td>
        <td width=\"128\" style='background-color:#FFF283;border-radius: 3px; -moz-border-radius: 3px; -webkit-border-radius: 3px;border: 1px solid #FFA500;'>
            <table width=\"100%\" class=\"noborder\" style=\"background:transparent\">
                <tr>
                    <td width=\"80%\" class=\"nobborder\" style=\"text-align:center;\">
                    <b>"._("Medium")."&nbsp;&nbsp;</b>
                    </td>
                    <td width=\"20%\" class=\"checkinfo nobborder\">
                    <input id=\"checkboxM\" type=\"checkbox\" onclick=\"toggle_vulns('checkboxM')\" checked>
                    </td>
                </tr>
            </table>
         </td>
        <td width=\"128\" style='background-color:#FFFFC0;border-radius: 3px; -moz-border-radius: 3px; -webkit-border-radius: 3px;border: 1px solid #FFD700;'>
            <table width=\"100%\" class=\"noborder\" style=\"background:transparent\">
                <tr>
                    <td width=\"80%\" class=\"nobborder\" style=\"text-align:center;\">
                    <b>"._("Low")."&nbsp;&nbsp;</b>
                    </td>
                    <td width=\"20%\" class=\"checkinfo nobborder\">
                    <input id=\"checkboxL\" type=\"checkbox\" onclick=\"toggle_vulns('checkboxL')\" checked></td>
                    </td>
                </tr>
            </table>
        </td>
        <td width=\"132\" style='background-color:#FFFFE3;border-radius: 3px; -moz-border-radius: 3px; -webkit-border-radius: 3px;border: 1px solid #F0E68C;'>
            <table width=\"100%\" class=\"noborder\" style=\"background:transparent\">
                <tr>
                    <td width=\"80%\" class=\"nobborder\" style=\"text-align:center;\">
                    <b>"._("Info")."&nbsp;&nbsp;</b>
                    </td>
                    <td width=\"20%\" class=\"checkinfo nobborder\">
                    <input id=\"checkboxI\" type=\"checkbox\" onclick=\"toggle_vulns('checkboxI')\" checked>
                    </td>
                </tr>
            </table>
        </td></tr>";

    $htmldetails .= "</form>";

    $perms_where = (Session::get_ctx_where() != "") ? " AND res.ctx in (".Session::get_ctx_where().")" : "";

    $dbconn->SetFetchMode(ADODB_FETCH_BOTH);

    if ($ipl=="all")
    {
        $query ="select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res
                    where falsepositive='N'
                    $perms_where";
    }
    else if (!empty($ipl) && !empty($ctx))
    {
        $query = "select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res
                    where falsepositive='N'
                    and res.hostIP='$ipl'
                    and res.ctx=UNHEX('$ctx')
                    $perms_where";
    }
    else if(!empty($scantime) && !empty($key))
    {
        $query = "select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res, vuln_nessus_latest_reports rep
                    where res.falsepositive='N'
                    and res.scantime='$scantime'
                    and res.hostIP=rep.hostIP
                    and res.ctx=rep.ctx
                    and res.username=rep.username
                    and res.sid=rep.sid
                    $perms_where
                    and rep.report_key='$key'";
    }

    $result = $dbconn->execute($query);

    $maxpag = 20;
    $hasta  = $pag*$maxpag;
    $desde  = $hasta - $maxpag;
    $hi = 0;

    while($result->fields)
    {
        $hostip  = $result->fields['hostIP'];
        $hostctx = $result->fields['ctx'];

        if(Session::hostAllowed_by_ip_ctx($dbconn, $hostip, $hostctx))
        {
            $host_id = key(Asset_host::get_id_by_ips($dbconn, $hostip, $hostctx));

            if(valid_hex32($host_id))
            {
                $hostname = Asset_host::get_name_by_id($dbconn, $host_id);
            }
            else
            {
                $hostname = _('unknown');
            }

            if ($desde <= $hi && $hi < $hasta)
            {
                $ips_in_range[$hostip.";".$hostctx] = $hostname;
            }

            $hi++;
        }

        $result->MoveNext();
    }

    foreach ($ips_in_range as $host_ip_ctx => $hostname)
    {
        list($h_ip, $h_ctx) = explode(";", $host_ip_ctx);

        if ($output == "full")
        {
            $tmp_host = "<a href='#$host_ip_ctx' id='$h_ip;$hostname' ctx='$h_ctx' class='anchor_link HostReportMenu'>$h_ip</a>";
        }
        else
        {
            $tmp_host = $h_ip;
        }

        $htmldetails .= "<tr>
        <td>&nbsp;$tmp_host</td><td>$hostname&nbsp;</td>";

        $prevrisk = 0;

        if(!empty($scantime) && !empty($key))
        {
            $query2 = "select count(*) as count,risk from (select distinct res.port, res.protocol, res.app, res.scriptid, res.risk, res.msg, res.hostIP
                from vuln_nessus_latest_results res, vuln_nessus_latest_reports rep
                where res.falsepositive='N'
                and res.scantime='$scantime'
                and res.hostIP=rep.hostIP
                and res.ctx=rep.ctx
                and res.username=rep.username
                and res.sid=rep.sid
                and res.hostIP='$h_ip'
                and res.ctx=UNHEX('$h_ctx')
                $perms_where
                and rep.report_key='$key') as t group by risk";
        }
        else
        {
            $query2 = "select count(*) as count,risk from (select distinct port, protocol, app, scriptid, risk, msg, hostIP
                from vuln_nessus_latest_results res where falsepositive='N' and hostIP='$h_ip' and ctx=UNHEX('$h_ctx')) as t group by risk";
        }

        $drawtable = 0;

        $result2 = $dbconn->execute($query2);

        $arisk = array();
        while($result2->fields)
        {
            $riskcount = $result2->fields['count'];
            $risk      = $result2->fields['risk'];

            if ($risk == 4)
            {
                $arisk[3] +=  $riskcount;
            }
            else if ($risk == 5)
            {
                $arisk[6] +=  $riskcount;
            }
            else
            {
                $arisk [$risk] = $riskcount;
            }

            $result2->MoveNext();
        }

        $lsrisk = array('1','2','3','6','7');

        foreach ($lsrisk as $lrisk)
        {
            if($arisk[$lrisk] != '')
            {
                $drawtable=1;
                $htmldetails .= "<td><a href=\"#".$h_ip."_".$h_ctx."_".$lrisk."\" class='anchor_link'>$arisk[$lrisk]</a></td>";
            }
            else
            {
                $htmldetails .= "<td>-</td>";
            }
        }

        if ($drawtable == 0)
        {
            $htmldetails .= "<td>-</td><td>-</td><td>-</td><td>-</td><td>-</td>";
        }

      $htmldetails .= "</tr>";
   }

   if ($hi>=$maxpag)
   {
     // pagination
     $first    = "<span style=\"color: #626262\"><< "._("First")."</span>";
     $previous = "<span style=\"color: #626262\">< "._("Previous")."  </span>";


   	 $url = str_replace("&amp;", "&", Util::htmlentities(preg_replace("/\&pag=\d+|\&chks=[tf]+/","",$_SERVER["QUERY_STRING"])));

     if ($pag>1)
     {
        $first = "<a href='javascript:;' onclick=\"jumptopage('?$url&pag=1')\" style='padding:0px 5px 0px 5px'>"._("<< First")."</a>";
        $previous = "<a href='javascript:;' onclick=\"jumptopage('?$url&pag=".($pag-1)."')\" style='padding:0px 5px 0px 5px'>"._("< Previous")."</a>";
     }

     $htmldetails .= "<tr><td colspan=11 class='nobborder' style='text-align:center'>";
   	 $tp = intval($hi/$maxpag); $tp += ($hi % $maxpag == 0) ? 0 : 1;
     $htmldetails .= $first." ".$previous;
     $pbr = 1;

   	 for ($p=1;$p<=$tp;$p++)
   	 {
   	    $pg = ($p==$pag) ? "<b>$p</b>" : $p;

        $htmldetails .= "<a href='javascript:;' onclick=\"jumptopage('?$url&pag=$p')\" style='padding:0px 5px 0px 5px'>$pg</a>";

        if ($pbr++ % 30 == 0)
        {
            $htmldetails .= "<br>";
        }
   	 }

     $next = "<span style=\"color:#626262\">  "._("Next")." ></span>";
     $last = "<span style=\"color:#626262\"> "._("Last")." >></span>";

     if ($pag<$tp)
     {
        $next = "<a href='javascript:;' onclick=\"jumptopage('?$url&pag=".($pag+1)."')\" style='padding:0px 5px 0px 5px'>"._("Next >")."</a>";
        $last = "<a href='javascript:;' onclick=\"jumptopage('?$url&pag=".$tp."')\" style='padding:0px 5px 0px 5px'>"._("Last >>")."</a>";
     }

     $htmldetails .= $next." ".$last;
   	 $htmldetails .= "</td></tr>";
   }

   $htmldetails .= "</table><br>";

   return $htmldetails;
}

function origdetails()
{
    global $uroles, $user, $sid, $query_risk, $border, $report_id, $scantime, $scantype, $fp, $nfp, $filterip,
    $enableFP, $output, $sortby, $dbconn;
    global $treport, $ipl, $ips_in_range, $ctx, $key;

    $colors = array ("Critical" => "#FFCDFF", "High" => "#FFDBDB", "Medium" => "#FFF283", "Low" => "#FFFFC0", "Info" => "#FFFFE3");
    $images = array ("Critical" => "./images/risk1.gif", "High" => "./images/risk2.gif", "Medium" => "./images/risk3.gif", "Low" => "./images/risk6.gif", "Info" => "./images/risk7.gif");
    $levels = array ("Critical" => "1", "High" => "2", "Medium" => "3", "Low" => "6", "Info" => "7");

    $query_host = '';

    if ($filterip)
    {
        $query_host = " AND hostip='$filterip'";
    }

    echo "<form>";
    echo "<table id='c_false_positives' style=\"background:transparent;\">";
    echo "<tr><td style=\"text-align:left;\" class=\"nobborder\">";
    echo "<input id=\"checkboxFP\" type=\"checkbox\" onclick=\"showFalsePositives()\"> <span style=\"color:black\">"._("View false positives")."</span>";
    echo "</td><td class=\"nobborder\" style=\"text-align:center;\">";

    // print the icon legend
    if ($enableFP)
    {
         echo "<img alt='True' src='images/true.gif' border=0 align='absmiddle'> - "._("True result")."&nbsp;&nbsp;";
         echo "<img alt='False' src='images/false.png' border=0 align='absmiddle'> - "._("False positive result")."&nbsp;&nbsp;";
    }

    echo "<img alt='Info' src='images/info.png' border=0 align='absmiddle'><span> - "._("Additional information is available")."</span>";
    echo "</td></tr></table>";
    echo "</form>";
    echo "<br>";

    $perms_where = (Session::get_ctx_where() != "") ? " AND res.ctx in (".Session::get_ctx_where().")" : "";

    if ($ipl=="all")
    {
        $query ="select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res
                    where falsepositive='N'
                    $perms_where";
    }
    else if (!empty($ipl) && !empty($ctx))
    {
        $query = "select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res
                    where falsepositive='N'
                    and res.hostIP='$ipl'
                    and res.ctx=UNHEX('$ctx')
                    $perms_where";
    }
    else if(!empty($scantime) && !empty($key))
    {
        $query = "select distinct res.hostIP, HEX(res.ctx) as ctx
                    from vuln_nessus_latest_results res, vuln_nessus_latest_reports rep
                    where res.falsepositive='N'
                    and res.scantime='$scantime'
                    and res.hostIP=rep.hostIP
                    and res.ctx=rep.ctx
                    and res.username=rep.username
                    and res.sid=rep.sid
                    $perms_where
                    and rep.report_key='$key'";
    }

    $resultp = $dbconn->execute($query);

    $host_range = array_keys($ips_in_range);
    while($resultp->fields)
    {
        $hostip  = $resultp->fields['hostIP'];
        $hostctx = $resultp->fields['ctx'];

        $host_id = key(Asset_host::get_id_by_ips($dbconn, $hostip, $hostctx));

        if(valid_hex32($host_id))
        {
            $hostname = Asset_host::get_name_by_id($dbconn, $host_id);
        }
        else
        {
            $hostname = _('unknown');
        }

        if (in_array($hostip.";".$hostctx ,$host_range))
        {
            echo "<div class='hostip'>";

            echo "<div id='host_report_link'><a name='$hostip;$hostctx' href='javascript:;' id='$hostip;$hostname' ctx='$hostctx' class='HostReportMenu'>$hostip - $hostname</a></div>";

            echo "<table id='c_reported_ports' summary=\"$hostip - "._("Reported Ports")."\">";

            echo "<tr><th colspan='2'>"._("Reported Ports")."</th></tr>";

            if(!empty($scantime) && !empty($key))
            {
                $query = "select distinct res.port, res.protocol
                from vuln_nessus_latest_results res, vuln_nessus_latest_reports rep
                where res.falsepositive='N'
                and res.scantime='$scantime'
                and res.hostIP=rep.hostIP
                and res.ctx=rep.ctx
                and res.username=rep.username
                and res.sid=rep.sid
                and res.hostIP='$hostip'
                and res.ctx='$hostctx'
                and rep.report_key='$key') as t group by risk";
            }
            else
            {
                $query ="select distinct res.port, res.protocol
                            from vuln_nessus_latest_results res
                            where hostip='$hostip' and ctx=UNHEX('$hostctx') AND port > '0' ORDER BY port ASC";
            }

            $result1 = $dbconn->execute($query);

            $k   = 1;
            $pos = '';

            if (! $result1->fields)
            {
                print "<tr><td>"._("No reported ports found")."</td></tr>";
            }
            else
            {
                while($result1->fields)
                {
                    $port  = $result1->fields['port'];
                    $proto = $result1->fields['protocol'];

                    if($k % 2)
                    {
                        echo "<tr><td>$port/$proto</td>";
                        $pos = "open";
                    }
                    else
                    {
                        echo "<td>$port/$proto</td></tr>";
                        $pos = "closed";
                    }

                    $k++;
                    $result1->MoveNext();
                } // end while

                // close up the table
                if($pos!="closed")
                {
                    echo "<td>&nbsp;</td></tr>";
                }
            }

        echo "</table><br/>";

        echo "<table id='c_vulns' summary='$hostip - risks'><tr>";
        echo "<th class='th_vuln_name'>"._("Vuln Name")."</th>";
        echo "<th class='th_vuln_id'>"._("VulnID")."</th>";
        echo "<th class='th_vuln_service'>"._("Service")."</th>";
        echo "<th class='th_vuln_severity'>"._("Severity")."</th>";
        echo "</tr>";

        if(!empty($scantime) && !empty($key))
        {
            $query = "select res.result_id, res.service, res.risk, res.falsepositive, res.scriptid, v.name, res.msg, rep.sid
                    from vuln_nessus_latest_results AS res LEFT JOIN vuln_nessus_plugins AS v ON v.id=res.scriptid, vuln_nessus_latest_reports rep
                    where res.msg<>''
                    and res.scantime='$scantime'
                    and res.hostIP=rep.hostIP
                    and res.ctx=rep.ctx
                    and res.hostIP='$hostip'
                    and res.ctx=UNHEX('$hostctx')
                    and res.username=rep.username
                    and res.sid=rep.sid
                    and rep.report_key='$key'";
        }
        else
        {
            $query = "select res.result_id, res.service, res.risk, res.falsepositive, res.scriptid, v.name, res.msg, rep.sid
                FROM vuln_nessus_latest_results res LEFT JOIN vuln_nessus_plugins AS v ON v.id=res.scriptid, vuln_nessus_latest_reports rep
                WHERE
                res.hostIP=rep.hostIP
                and res.ctx=rep.ctx
                and res.username=rep.username
                and res.sid=rep.sid
                and res.hostIP='$hostip'
                and res.ctx=UNHEX('$hostctx')
                and msg<>''";
        }

        $query.=" group by risk, port, protocol, app, scriptid, msg  order by risk";

        $result1 = $dbconn->execute($query);

        $arrResults = array();

        while($result1->fields)
        {
            $result_id     = $result1->fields['result_id'];
            $service       = $result1->fields['service'];
            $risk          = $result1->fields['risk'];
            $falsepositive = $result1->fields['falsepositive'];
            $scriptid      = $result1->fields['scriptid'];
            $pname         = $result1->fields['name'];
            $msg           = $result1->fields['msg'];
            $sid           = $result1->fields['sid'];

            $tmpport1=preg_split("/\(|\)/",$service);

            if (sizeof($tmpport1)==1)
            {
                $tmpport1[1]=$tmpport1[0];
            }

            $tmpport2 = preg_split("/\//",$tmpport1[1]);

            $service_num   = $tmpport2[0];
            $service_proto = $tmpport2[1];

            $arrResults[] = array($service_num,
                $service_proto,
                $service,
                $risk,
                $falsepositive,
                $result_id,
                $msg,
                $scriptid,
                $pname,
                $sid
            );

            $result1->MoveNext();
        }

        if(empty($arrResults))
        {
            // empty, print out message
            echo "<tr><td colspan='4'>"._("No vulnerability results matching this reports filtering criteria were found").".</td></tr>";
        }

        foreach ($arrResults as $arrkey => $value)
        {
            list($service_num,
               $service_proto,
               $service,
               $risk,
               $falsepositive,
               $resid,
               $msg,
               $scriptid,
               $pname,
               $sid) = $value;

            $msg = preg_replace("/^[ \t]*/","",$msg);
            $msg = preg_replace("/[\n\r]/","<br>",$msg);
            $msg = str_replace('\n', "<br>", $msg);

            $tmprisk = getrisk($risk);

            $msg = preg_replace("/^\<br\>/i","",str_replace("\\r", "", $msg));
            $msg = preg_replace("/(Insight|CVSS Base Score|Vulnerability Detection Method|Vulnerability Detection Result|CVSS Base Vector|Solution|Summary|Details|Overview|Synopsis|Description|See also|Plugin output|References|Vulnerability Insight|Vulnerability Detection|Impact|Impact Level|Affected Software\/OS|Fix|Information about this scan)\s*:/","<b>\\1:</b>",$msg);
            $msg= str_replace("&amp;","&", $msg);

            //Parse links
            $msg = explode("<b>References:</b>", $msg);
            $references = array_pop($msg);
            $msg = implode("<b>References:</b>", $msg);

            // Make CVEs linkables
            $msg = preg_replace_callback("/CVE\-\d{4}\-\d{4,}+/i", get_cve_link, $msg);

            // Make references linkables (This function must be called first)
            $msg .= "<b>References:<b/>".preg_replace_callback("/https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b([-a-zA-Z0-9()@:%_\+.~#?&\/\/=]*)/i", get_vuln_reference_link, $references);

            // output the table cells
            $anchor = $hostip."_".$hostctx."_".$levels[$tmprisk];

            $pname = ($pname!="") ? $pname : _("No name");

            echo "<tr ".(($falsepositive=='Y')? "class=\"trsk risk$risk fp\"" : "class=\"trsk risk$risk\"")."style=\"background-color:".$colors[$tmprisk].(($falsepositive=='Y')? ";display:none;" : "")."\">";
            //echo "<tr>";

            echo "<td class='td_vuln_name'><b>".$pname."</b></td>";

            echo "<td class='td_vuln_id'>$scriptid</td>";
            ?>
                <td class="td_vuln_service"><?php echo $service; ?></td>
                <td class="td_vuln_severity">
                    <?php echo $tmprisk; ?><img align="absmiddle" src="<?php echo $images[$tmprisk] ?>" style="border: 1px solid ; width: 25px; height: 10px; margin-left: 3%">
                </td>
            </tr>

            <?php
            echo "<tr ".(($falsepositive=='Y')? "class=\"trsk risk$risk fp\"" : "class=\"trsk risk$risk\"")."style=\"background-color:".$colors[$tmprisk].(($falsepositive=='Y')? ";display:none;" : "")."\">";
                ?>
                <td style="padding:3px 0px 3px 6px; text-align:left; word-break: break-word;">
                    <a class="msg" name="<?php echo $resid ?>"></a>
                    <a name="<?php echo $anchor ?>"></a>
                    <?php echo $msg; ?>
                    <br><br>
                    <?php
                    if($scriptid != "0")
                    {
                        ?>
                        <!--Add link to popup with Script Info-->
                        <div lid="<?php echo $scriptid;?>" style="text-decoration:none;display:inline" class="scriptinfo"><img alt="Info" src="images/info.png" border=0></div>
                        <?php
                    }

                    $tmpu = array();
                    $url = "";
                    foreach ($_GET as $kget => $vget)
                    {
                        if($kget!="pluginid" && $kget!="nfp" && $kget!="fp")
                        {
                            $tmpu[] = Util::htmlentities($kget)."=".urlencode($vget);
                        }
                    }

                    $url = implode("&",$tmpu);

                    if ($falsepositive=="Y")
                    {
                        ?>
                        <a href='javascript:;' onclick="jumptopage('<?php echo $_SERVER['SCRIPT_NAME'].'?'.$url?>&nfp=<?php echo $resid?>')">
                            <img alt="<?php echo _("Clear false positive")?>" src='images/false.png' title='<?php echo _("Clear false positive")?>' border='0' />
                        </a>
                        <?php
                    }
                    else
                    {
                        ?>
                        <a href='javascript:;' onclick="jumptopage('<?php echo $_SERVER['SCRIPT_NAME'].'?'.$url?>&fp=<?php echo $resid?>')">
                            <img alt="<?php echo _("Mark as false positive")?>" src='images/true.gif' title='<?php echo _("Mark as false positive")?>' border='0' />
                        </a>
                        <?php
                    }

                    $pticket = "ref=Vulnerability&title=".urlencode($pname)."&priority=1&ip=".urlencode($hostip)."&port=".urlencode($service_num).
                    "&nessus_id=".urlencode($scriptid)."&risk=".urlencode($tmprisk)."&type=".urlencode("Vulnerability");


                    echo "<a title=\""._("New ticket")."\" class=\"greybox\" href=\"../incidents/newincident.php?$pticket\"><img style=\"padding-bottom:2px;\" src=\"../pixmaps/script--pencil.png\" border=\"0\" alt=\"i\" width=\"12\"></a>&nbsp;&nbsp;";
                    ?>
                </td>

                <?php
                $plugin_info = $dbconn->execute("SELECT t2.name as pfamily, t3.name as pcategory, t1.cve_id, t1.created, t1.modified
                        FROM vuln_nessus_plugins t1
                        LEFT JOIN vuln_nessus_family t2 on t1.family=t2.id
                        LEFT JOIN vuln_nessus_category t3 on t1.category=t3.id
                        WHERE t1.id='$scriptid'");

                $pfamily    = $plugin_info->fields['pfamily'];
                $pcategory  = $plugin_info->fields['pcategory'];
                $pcreated   = $plugin_info->fields['created'];
                $pmodified  = $plugin_info->fields['modified'];
                $cve_id     = preg_replace_callback("/CVE\-\d{4}\-\d{4,}+/i", get_cve_link, $plugin_info->fields['cve_id']);
                ?>

                <td colspan="3" valign="top" class="c_plugin_details">
                    <?php
                    $plugin_details = '';
                    if ($pfamily!='')    { $plugin_details .= '<b>Family name:</b> '.$pfamily.'<br><br>';}
                    if ($pcategory!='')  { $plugin_details .= '<b>Category:</b> '.$pcategory.'<br><br>'; }
                    if ($pcreated!='')   { $plugin_details .= '<b>Created:</b> '.$pcreated.'<br><br>'; }
                    if ($pmodified!='')  { $plugin_details .= '<b>Modified:</b> '.$pmodified.'<br><br>'; }
                    if ($cve_id != '')   { $plugin_details .= '<b>CVEs:</b> '.$cve_id.'<br><br>'; }

                    echo $plugin_details;
                    ?>
                </td>
            </tr>
            <?php

            $result1->MoveNext();
        }

        echo "</table>";
        echo "</div>";

        }

        $resultp->MoveNext();
    }
}

$ips_in_range = array();

switch($disp)
{
    case "html":
        generate_results($output);
    break;

    default:
        generate_results($output);
    break;
}

echo "<br>";

$dbconn->close();
?>
<br/><br/><br/>
</body>
</html>