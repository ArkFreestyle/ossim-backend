<?php
/**
* Incident_ticket.inc
*
* File Incident_ticket.inc is used to:
*   - Task 1 (TO DO)
*   - Task 2 (TO DO)
*   - Task 3 (TO DO)
*
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
* @package    ossim-framework\Incidents
* @autor      AlienVault INC
* @license    http://www.gnu.org/licenses/gpl-2.0.txt
* @copyright  2003-2006 ossim.net
* @copyright  2007-2013 AlienVault
* @link       https://www.alienvault.com/
*/

/**
* Function list:
* - Incident_ticket()
* - get_id()
* - get_incident_id()
* - get_status()
* - get_priority()
* - get_user()
* - get_description()
* - get_action()
* - get_in_charge()
* - get_transferred()
* - get_attachment()
* - get_date()
* - get_list()
* - insert()
* - update_priority()
* - delete()
* - change_status()
* - mail_subscription()
* - get_email_template_values()
* - get_email_template()
* - build_email_template()
* - save_email_template()
*/

class Incident_ticket
{
    public $id;


    public $incident_id;


    public $date;


    public $status;


    public $priority;


    public $user;


    public $description;


    public $action;


    public $in_charge;


    public $transferred;


    public function __construct($id, $incident_id, $date, $status, $priority, $user, $description, $action, $in_charge, $transferred)
    {
        $this->id = $id;
        $this->incident_id = $incident_id;
        $this->date        = $date;
        $this->status      = $status;
        $this->priority    = $priority;
        $this->user        = $user;
        $this->description = $description;
        $this->action      = $action;
        $this->in_charge   = $in_charge;
        $this->transferred = $transferred;
    }


    public function get_id()
    {
        return $this->id;
    }


    public function get_incident_id()
    {
        return $this->incident_id;
    }


    public function get_status()
    {
        return $this->status;
    }


    public function get_priority()
    {
        return $this->priority;
    }


    public function get_user()
    {
        return $this->user;
    }


    public function get_description()
    {
        $txt = trim($this->description);

        if(preg_match("/\A<!--wiki-->/", $txt))
        {
            $wiki = new Wikiparser();

            $txt  = preg_replace("/<!--wiki-->/","", $txt);
            $txt  = Util::htmlentities($txt, ENT_NOQUOTES);

            if(!empty($txt))
            {
                $txt = $wiki->parse($txt);
            }
        }
        else
        {
            //This is to keep compatibility with ticket for versions < 4.1
            $txt  = Util::sanitize_xss($txt);
        }

        return $txt;
    }


    public function get_action()
    {
        $txt = trim($this->action);

        if(preg_match("/\A<!--wiki-->/", $txt))
        {
            $wiki = new Wikiparser();

            $txt  = preg_replace("/<!--wiki-->/","", $txt);
            $txt  = Util::htmlentities($txt, ENT_NOQUOTES);

            if(!empty($txt))
            {
                $txt = $wiki->parse($txt);
            }
        }
        else
        {
            //This is to keep compatibility with ticket for versions < 4.1
            $txt = Util::sanitize_xss($txt);
        }

        return $txt;
    }


    public function get_in_charge()
    {
        return $this->in_charge;
    }


    public function get_transferred()
    {
        return $this->transferred;
    }


    public function get_attachment($conn)
    {
        $attachment = Incident_file::get_list($conn, " WHERE incident_id = " . $this->incident_id . " AND incident_ticket = " . $this->id);

        if ($attachment)
        {
            return $attachment[0];
        }
        else
        {
            return FALSE;
        }
    }


    public function get_date()
    {
        return Util::timestamp2date($this->date);
    }


    public static function get_list($conn, $args)
    {
        $tz = Util::get_timezone();

        $list = array();
        $query = ossim_query("SELECT * FROM incident_ticket $args");

        $rs = $conn->Execute($query);

        if (!$rs)
        {
            Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }
        else
        {
            while (!$rs->EOF)
            {
                if($tz != 0)
                {
                    $rs->fields['date'] = gmdate('Y-m-d H:i:s', Util::get_utc_unixtime($rs->fields['date'])+3600*$tz);
                }

                $list[] = new self($rs->fields['id'], $rs->fields['incident_id'], $rs->fields['date'], $rs->fields['status'], $rs->fields['priority'], $rs->fields['users'], $rs->fields['description'], $rs->fields['action'], $rs->fields['in_charge'], $rs->fields['transferred']);

                $rs->MoveNext();
            }
        }

        return $list;
    }


    /*
     * @param $tags Array with the tag IDs related to this incident
     */
    public static function insert($conn, $incident_id, $status, $priority, $user, $description = '', $action = '', $transferred = NULL, $tags = array(), $attachment = array(), $sendmail = true)
    {
        $tz   = Util::get_timezone();
        $date = ($tz == 0) ? date('Y-m-d H:i:s') : gmdate("Y-m-d H:i:s");

        // Get previous in charge
        $query = 'SELECT in_charge FROM incident WHERE id=?';
        $in_charge = $conn->GetOne($query, array($incident_id));

        if ($in_charge === FALSE)
        {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        // Insert new ticket
        $conn->StartTrans();
        $ticket_id = $conn->GetOne('SELECT max(id)+1 FROM incident_ticket');
        $ticket_id = (intval($ticket_id) == 0) ? 1 : $ticket_id;

        $query = 'INSERT INTO incident_ticket (id, incident_id, status, priority, users, description, action, transferred, in_charge, date) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)';

        // Transferred user is the new in_charge user
        $params = array(
            $ticket_id,
            $incident_id,
            $status,
            $priority,
            $user,
            $description,
            $action,
            $transferred,
            $in_charge,
            $date
        );

        $conn->Execute($query, $params);


        // Reflect changes in the Incident
        if ($transferred)
        {
            $in_charge = $transferred;
        }

        $query = 'UPDATE incident SET priority=?, last_update=?, in_charge=?, status=? WHERE id=?';

        $params = array(
            $priority,
            $date,
            $in_charge,
            $status,
            $incident_id
        );

        $conn->Execute($query, $params);


        //Update incident tags
        $query = 'DELETE FROM incident_tag WHERE incident_id=?';
        $conn->Execute($query, array($incident_id));

        $query = 'INSERT INTO incident_tag (tag_id, incident_id) VALUES (?, ?)';
        foreach($tags as $tag)
        {
            $conn->Execute($query, array($tag, $incident_id));
        }


        //User in charge is subscribed by default
        if ($transferred)
        {
            Incident::insert_subscription($conn, $incident_id, $transferred);
        }

        // Finish transaction
        $_result = $conn->CompleteTrans();

        if (!$_result)
        {
            Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());

            return FALSE;
        }

        if ($attachment)
        {
            Incident_file::insert($conn, $incident_id, $ticket_id, $attachment["name"], $attachment["type"], $attachment["content"]);
        }


        // Mail ticket to subscribers
        if ($sendmail !== false) {
            self::mail_subscription($conn, $incident_id, $ticket_id, 'UPDATE_TICKET');
        }

        $count_tickets = self::count_incident_tickets($conn, $incident_id);

        if($count_tickets == 1)
        {
            Webservice::send_ticket_to_platform($conn, $incident_id, $description);
        }

        if ($status == "Closed") {
                Log_action::log(108, array($incident_id,$user));
        } else {
                Log_action::log(50, array($ticket_id, $incident_id, $user));
        }


        return TRUE;
    }


    public static function update_priority($conn, $id, $incident_id, $priority)
    {
        /* update priority */
        $id          = intval($id);
        $incident_id = intval($incident_id);
        $priority    = intval($priority);

        $query  = "UPDATE incident_ticket SET priority = ?, date = date WHERE id = ?";
        $params = array($priority, $id);

        if ($conn->Execute($query, $params) === FALSE)
        {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        /* get last id */
        $query  = "SELECT id, incident_id FROM incident_ticket WHERE incident_id = ? ORDER BY id DESC LIMIT 1";
        $params = array($incident_id);
        $rs     = $conn->Execute($query, $params);

        if (!$rs)
        {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        $max_id      = $rs->fields["id"];
        $incident_id = $rs->fields["incident_id"];

        /* if it's the last id, update global priority */
        if ($id == $max_id)
        {
            $query  = "UPDATE incident SET priority = ? WHERE id = ?";
            $params = array($priority, $incident_id);

            if ($conn->Execute($query, $params) === FALSE)
            {
                Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
            }
        }
    }


    public static function delete($conn, $ticket_id, $incident_id)
    {
        if (!security_class::ossim_valid($ticket_id, OSS_DIGIT))
        {
            $exp_msg = _('Invalid ticket ID');

            Av_exception::throw_error(Av_exception::USER_ERROR, $exp_msg);
        }

        $conn->StartTrans();

        $params = array($incident_id, $ticket_id);

        $conn->Execute("DELETE FROM incident_file WHERE incident_id=? AND incident_ticket=?", $params);
        $conn->Execute("DELETE FROM incident_tmp_email WHERE incident_id=? AND ticket_id=?", $params);

        $params = array($ticket_id);
        $conn->Execute("DELETE FROM incident_ticket WHERE id=?", $params);

        if (!$conn->CompleteTrans())
        {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        $files = glob("/usr/share/ossim/www/uploads/Incident-*-$ticket_id-*.*");
        array_map('unlink', $files);

        $username = Session::get_session_user();

        $infolog = array($ticket_id, $incident_id, $username);

        Log_action::log(51, $infolog);
    }


    public static function change_status($conn, $id)
    {
        $id    = intval($id);
        $query = "SELECT status FROM incident_ticket WHERE id = ?";

        $params = array($id);
        $rs     = $conn->Execute($query, $params);

        if (!$rs)
        {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        $status = $rs->fields["status"];

        if ($status == 'Open')
        {
            $query2 = "UPDATE incident_ticket SET status = 'Closed',
            date = date WHERE id = ?";
        }
        elseif ($status == 'Closed')
        {
            $query2 = "UPDATE incident_ticket SET status = 'False',
            date = date WHERE id = ?";
        }
        elseif ($status = 'False')
        {
            $query2 = "UPDATE incident_ticket SET status = 'Open',
            date = date WHERE id = ?";
        }

        $params = array($id);

        if (isset($query2))
        {
            if ($conn->Execute($query2, $params) === FALSE)
            {
                Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
            }
        }
    }


    public static function count_incident_tickets($conn, $ticket_id)
    {
        $query = ossim_query("SELECT count(id) as total FROM incident_ticket WHERE incident_id = ? ");

        $params = array($ticket_id);
        $rs     = $conn->Execute($query, $params);

        if (!$rs)
        {
            Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());

            return -1;
        }
        else
        {
            return $rs->fields["total"];
        }
    }

    public static function mail_subscription($conn, $incident_id, $ticket_id, $type) {

        //Adding user in charge to the mail list
        list($incident) = Incident::search($conn, array('incident_id' => $incident_id));

        $in_charge = $incident->get_in_charge();
        $subscribers = Incident::get_subscribed_users($conn, $incident_id, $in_charge);

        $subscriber_list = array();
        foreach($subscribers as $s) {
            $subscriber_list[] = $s['login'];
        }

        //Getting the current subscribers
        $subscriber_list = implode(";", $subscriber_list);

        $query = "REPLACE INTO incident_tmp_email (incident_id, ticket_id, type, subscribers) VALUES (?, ?, ?, ?)";
        $params = array($incident_id, $ticket_id, $type, $subscriber_list);

        if ($conn->Execute($query, $params) === FALSE) {
            Av_exception::throw_error(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }

        return TRUE;
    }


    /** Get subscribers from a incident ticket
     * @param $conn         object Database access object
     * @param $incident_id  string Incident ID
     * @param $ticket_id    string Ticket ID
     * @return array
     */
    public static function get_ticket_subscribers($conn, $incident_id, $ticket_id){
        $pro = Session::is_pro();

        $subscribers = array();

        //Getting subscribers
        $params = array($incident_id, $ticket_id);
        $query = "SELECT subscribers FROM incident_tmp_email WHERE incident_id = ? AND ticket_id = ?";

        $rs = $conn->Execute($query, $params);

        if (!$rs)
        {
            Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }
        else
        {
            $subscribers = explode(";", $rs->fields['subscribers']);
        }

        if (is_array($subscribers) && !empty($subscribers))
        {
            foreach ($subscribers as $subscriber){
                //User is an entity
                if ($pro && valid_hex32($subscriber)) {
                    $entity_users = Acl::get_all_users_by_entity($conn, $subscriber);

                    foreach ($entity_users as $entity_user) {
                        $subscribers[$entity_user] = $entity_user;
                    }
                } else {
                    $subscribers[$subscriber] = $subscriber;
                }
            }

            $users = array();
            $subscribers = "'".implode("','", $subscribers)."'";

            $query = "SELECT u.login, u.name, u.email, u.company, u.department FROM users u
                WHERE u.login in ($subscribers) 
                ORDER BY u.name";

            $rs = $conn->Execute($query);


            if (!$rs)
            {
                Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());
            }
            else
            {
                while (!$rs->EOF)
                {
                    $users[] = $rs->fields;
                    $rs->MoveNext();
                }
            }
        }
        return array_map("unserialize", array_unique(array_map("serialize", $users)));

    }


    /** Send an incident ticket by email
     * @param $conn         object Database access object
     * @param $incident_id  string Incident ID
     * @param $ticket_id    string Ticket ID
     * @param string $type  string Update type
     * @return bool
     */
    public static function mail_notification($conn, $incident_id, $ticket_id, $type = '')
    {
        $conf = $GLOBALS["CONF"];

        if (strtolower($conf->get_conf('tickets_send_mail')) == 'no')
        {
            return TRUE; // Do not generate emails
        }

        $error = '';
        $values      = self::get_email_template_values($conn, $incident_id, $ticket_id);
        $subscribed_users = self::get_ticket_subscribers($conn, $incident_id, $ticket_id);

        //Email
        if ($type == 'CREATE_INCIDENT')
        {
            $values['TICKET_DESCRIPTION']  = _("You're subscribed to this ticket or you're in charge");
        }
        elseif ($type == 'UPDATE_INCIDENT')
        {
            $values['TICKET_DESCRIPTION']  = _("User in charge has changed");
        }

        $subject_tpl = self::get_email_template('subject');
        $body_tpl    = self::get_email_template('body');

        //Attachments
        $attachments = self::get_attachment_to_mail($conn, $incident_id, $ticket_id);

        $subject     = self::build_email_template($subject_tpl, $values);
        $subject     = Util::html_entities2utf8($subject);

        $body        = self::build_email_template($body_tpl, $values);

        foreach($subscribed_users as $user)
        {
        	if ($user['email'] != '')
        	{
	            $res = Util::send_email($conn, $user['email'], $subject, $body, $attachments);

	            if (!$res)
	            {
	                $error.= _("Could not email ticket to the following address") . ": " . $user['email'] . "<br/>";
	            }
        	}
        }

        @unlink($attachments[0]['path']);

        return $error ? ossim_set_error($error) : TRUE;
    }

    public function get_attachment_to_mail($conn, $incident_id, $ticket_id)
    {
        $list  = array();

        $query  = ossim_query("SELECT * FROM incident_file WHERE incident_id = ? AND incident_ticket = ? ");
        $params = array($incident_id, $ticket_id);
        $rs     = $conn->Execute($query, $params);

        if (!$rs)
        {
            Av_exception::write_log(Av_exception::DB_ERROR, $conn->ErrorMsg());
        }
        else
        {
            if (!$rs->EOF)
            {
                $path   = "/tmp/attached_".$incident_id."_".$ticket_id;

                if (file_put_contents($path, $rs->fields["content"]) === FALSE)
                {
                    @unlink($path);
                }
                else
                {
                    $list[] = array (
                        "path"    => $path,
                        "name"    => $rs->fields["name"],
                        "type"    => $rs->fields["type"]
                   );
                }
            }
        }
        return $list;
    }


    public function get_email_template_values($conn, $incident_id, $ticket_id)
    {
        //
        // Ticket data
        //

        if (intval($ticket_id) > 0)
        {
            list($ticket) = self::get_list($conn, "WHERE incident_ticket.id = $ticket_id");

            $incident_id  = $ticket->get_incident_id();
            $values['ID'] = $incident_id;
            $values['TICKET_DESCRIPTION'] = $ticket->get_description();
            $values['TICKET_ACTION']      = $ticket->get_action();

            list($incident) = Incident::search($conn, array('incident_id' => $values['ID']));
            $login = $ticket->get_user();
        }
        else
        {
            $values['ID'] = $incident_id;
            $values['TICKET_DESCRIPTION'] = '';
            $values['TICKET_ACTION']      = '';

            list($incident) = Incident::search($conn, array('incident_id' => $values['ID']));

            $submitter = $incident->get_submitter();
            $user_data = explode("/", $submitter);
            $login     = (count($user_data) > 1) ? $user_data[1] : $user_data[0];
        }
        //
        // Incident data
        //

        $values['INCIDENT_NO']   = $incident->get_ticket();
        $values['TITLE']         = $incident->get_title();
        $values['PRIORITY_NUM']  = $incident->get_priority();
        $values['PRIORITY_STR']  = $incident->get_priority_string($values['PRIORITY_NUM']);
        $values['STATUS']        = $incident->get_status();
        $values['CLASS']         = $incident->get_ref();
        $values['TYPE']          = $incident->get_type();
        $values['CREATION_DATE'] = $incident->get_date();
        $values['LIFE_TIME']     = $incident->get_life_time();


        //
        // Ticket author details
        //

        if (valid_hex32($login))
        {
            $values['TICKET_AUTHOR_NAME']    = Acl::get_entity_name($conn, $login);
            $values['TICKET_AUTHOR_EMAIL']   = '';
            $values['TICKET_AUTHOR_DPTO']    = '';
            $values['TICKET_AUTHOR_COMPANY'] = '';
        }
        else
        {
            list($author) = Session::get_list($conn, "WHERE login = '$login'");

            if (!$author)
            {
                $values['TICKET_AUTHOR_NAME']    = _('Unknown');
                $values['TICKET_AUTHOR_EMAIL']   = _('Unknown');
                $values['TICKET_AUTHOR_DPTO']    = _('Unknown');
                $values['TICKET_AUTHOR_COMPANY'] = _('Unknown');
            }
            else
            {
                $values['TICKET_AUTHOR_NAME']    = $author->name;
                $values['TICKET_AUTHOR_EMAIL']   = $author->email;
                $values['TICKET_AUTHOR_DPTO']    = $author->department;
                $values['TICKET_AUTHOR_COMPANY'] = $author->company;
            }
        }

        $values['TICKET_INVERSE_HISTORY'] = '';
        $values['TICKET_HISTORY']         = '';

        if (intval($ticket_id) > 0)
        {
            //
            // Ticket history
            //
            $history = '';

            $list = self::get_list($conn, "WHERE incident_ticket.incident_id = $incident_id AND incident_ticket.id < '$ticket_id' ORDER BY incident_ticket.date");

            foreach($list as $ticket)
            {
                $login = $ticket->get_user();

                list($author) = Session::get_list($conn, "WHERE login = '$login'");

                if (!$author)
                {
                    $author_name = _("Unknown");
                    $author_mail = _("Unknown");
                }
                else
                {
                    $author_name = $author->get_email();
                    $author_mail = $author->get_email();
                }

                $history.= $ticket->get_date() . ' <a href="mailto://' . $author_mail . '">' . $author_name. "</a><br />" . "<hr>" . _("Description") . ":<br />" . $ticket->get_description() . "<br /><br />" . _("Action") . ":<br />" . $ticket->get_action() . "<br /><br />";
            }

            $values['TICKET_HISTORY'] = $history;
            $history = '';

            $list = self::get_list($conn, "WHERE incident_ticket.incident_id = $incident_id AND id < '$ticket_id' ORDER BY incident_ticket.date DESC");

            foreach($list as $ticket)
            {
                $login = $ticket->get_user();
                list($author) = Session::get_list($conn, "WHERE login = '$login'");

                if (!$author)
                {
                    $author_name = _('Unknown');
                    $author_mail = _('Unknown');
                }
                else
                {
                    $author_name = $author->get_email();
                    $author_mail = $author->get_email();
                }
                $history.= $ticket->get_date() . ' <a href="mailto://' . $author_mail . '">' . $author_name. "</a><br />" . "<hr>" . _("Description") . ":<br />" . $ticket->get_description() . "<br /><br />" . _("Action") . ":<br />" . $ticket->get_action() . "<br /><br />";
            }

            $values['TICKET_INVERSE_HISTORY'] = $history;
        }

        //
        // Related Incident tags
        //
        $incident_tag = new Incident_tag($conn);
        $tags = array();
        foreach($incident->get_tags() as $tag)
        {
            $tags[] = $incident_tag->get_tag_info($tag, 'name');
        }

        $values['TAGS'] = implode(", ", $tags);

        //
        // Extra incident info
        //
        $info = '';

        switch ($values['CLASS'])
        {
            case 'Event':
            case 'Alarm':
                $method = $values['CLASS'] == 'Event' ? 'get_events' : 'get_alarms';

                list($obj) = $incident->$method($conn);

                if ($obj)
                {
                    if ($obj->get_src_ips())
                    {
                        $info .= "<strong>"._('Source IPs') . ': </strong>' . $obj->get_src_ips() . "<br/>";
                    }

                    if ($obj->get_src_ports())
                    {
                        $info .= "<strong>"._('Source Ports') . ': </strong>' . $obj->get_src_ports() . "<br/>";
                    }

                    if ($obj->get_dst_ips())
                    {
                        $info .="<strong>"._('Dest. IPs') .     ': </strong>' . $obj->get_dst_ips() . "<br/>";
                    }

                    if ($obj->get_dst_ports())
                    {
                        $info .= "<strong>"._('Dest. Ports') .  ': </strong>' . $obj->get_dst_ports();
                    }
                }

            break;

            case 'Metric':
                list($obj) = $incident->get_metrics($conn);

                if ($obj)
                {
                    if ($obj->get_target())
                    {
                        $info .= "<strong>"._('Target') .       ': </strong>' . $obj->get_target() . "<br/>";
                    }

                    if ($obj->get_metric_type())
                    {
                        $info .= "<strong>"._('Metric Type') .  ': </strong>' . $obj->get_metric_type() . "<br/>";
                    }

                    if ($obj->get_metric_value())
                    {
                        $info .= "<strong>"._('Metric Value') . ': </strong>' . $obj->get_metric_value();
                    }
                }
            break;

            case 'Anomaly':
                list($obj) = $incident->get_anomalies($conn);

                if ($obj)
                {
                    if ($obj->get_anom_type())
                    {
                        $info .= "<strong>"._('Anomaly type') . ': </strong>' . strtoupper($obj->get_anom_type()) . "<br/>";
                    }

                    if ($obj->get_ip())
                    {
                        $info .= "<strong>"._('IP') .           ': </strong>' . $obj->get_ip() . "<br/>";
                    }

                    if ($obj->get_data_orig())
                    {
                        $info .= "<strong>"._('Previous') .     ': </strong>' . $obj->get_data_orig() . "<br/>";
                    }

                    if ($obj->get_data_new())
                    {
                        $info .= "<strong>"._('New') .          ': </strong>' . $obj->get_data_new();
                    }
                }
            break;

            case 'Vulnerability':
                list($obj) = $incident->get_vulnerabilities($conn);

                if ($obj)
                {
                    if ($obj->get_ip())
                    {
                        $info .= "<strong>"._('IP')          .   ': </strong>' . $obj->get_ip() . "<br/>";
                    }

                    if ($obj->get_port())
                    {
                        $info .= "<strong>"._('Port')        .   ': </strong>' . strtoupper($obj->get_port()) . "<br/>";
                    }

                    if ($obj->get_nessus_id())
                    {
                        $info .=  "<strong>"._('Scanner ID')  .  ': </strong>' . $obj->get_nessus_id() . "<br/>";
                    }

                    if ($obj->get_risk())
                    {
                        $info .= "<strong>"._('Risk')         .  ': </strong>' . $obj->get_risk() . "<br/>";
                    }

                    if ($obj->get_description())
                    {
                        $info .= "<strong>"._('Description')  .  ': </strong>' . nl2br($obj->get_description());
                    }
                }
            break;
        }

        $values['EXTRA_INFO'] = $info;

        //
        // In charge details
        //
        $login = $incident->get_in_charge();

        if (valid_hex32($login))
        {
            $e_data = Acl::get_entity($conn, $login, FALSE, FALSE);

            $values['IN_CHARGE_LOGIN']   = $e_data['id'];
            $values['IN_CHARGE_NAME']    = $e_data['name'];
            $values['IN_CHARGE_EMAIL']   = "";
            $values['IN_CHARGE_DPTO']    = "";
            $values['IN_CHARGE_COMPANY'] = "";
        }
        else
        {
            list($in_charge) = Session::get_list($conn, "WHERE login = '$login'", '', TRUE);

            if (!$in_charge)
            {
                $in_charge_name = _('Unknown');
                $in_charge_mail = _('Unknown');
                $in_charge_dep  = _('Unknown');
                $in_charge_com  = _('Unknown');
            }
            else
            {
                $in_charge_name = $in_charge->get_name();
                $in_charge_mail = $in_charge->get_email();
                $in_charge_dep  = $in_charge->get_department();
                $in_charge_com  = $in_charge->get_company();
            }

            $values['IN_CHARGE_LOGIN']   = $login;
            $values['IN_CHARGE_NAME']    = $in_charge_name;
            $values['IN_CHARGE_EMAIL']   = $in_charge_mail;
            $values['IN_CHARGE_DPTO']    = $in_charge_dep;
            $values['IN_CHARGE_COMPANY'] = $in_charge_com;
        }

        return $values;
    }


    public static function get_email_template($type)
    {
        $default_subject_template = '[alienvault-incident] PRIORITY_STR: TITLE';
        $default_body_template = '<h1>Incident details</h1>
<hr>
<b>Title:</b> INCIDENT_NO - TITLE<br />
<b>Status:</b> STATUS<br />
<b>Type:</b> CLASS - TYPE<br />
<b>Priority:</b> PRIORITY_NUM (PRIORITY_STR)<br />
<b>In charge:</b> <a href="mailto://IN_CHARGE_EMAIL">IN_CHARGE_NAME</a><br />
<b>Created:</b> CREATION_DATE (LIFE_TIME ago)<br />
<b>Tags:</b> TAGS<br />
<br />
<h1>Extra info:</h1>
EXTRA_INFO<br />
<br />
<h1>Ticket details</h1>
<hr>
<b>Author:</b> <a href="mailto://TICKET_AUTHOR_EMAIL">TICKET_AUTHOR_NAME</a>
<br />
TICKET_DESCRIPTION<br />
<br />
<b>Actions:</b><br />
<br />
TICKET_ACTION<br />
<br />
<b>Past tickets:</b><br />
<br />
TICKET_INVERSE_HISTORY<br />
<br />
';
        $conf = $GLOBALS["CONF"];

        if (!in_array($type, array('subject','body')))
        {
            $exp_msg = _("Invalid template type:")." ".$type;

            Av_exception::throw_error(Av_exception::USER_ERROR, $exp_msg);
        }

        $tpl = $conf->get_conf('email_' . $type . '_template');

        // There is no such conf option or it's empty
        if (!$tpl && $type == 'subject')
        {
            $tpl = $default_subject_template;
        }
        elseif (!$tpl && $type == 'body')
        {
            $tpl = $default_body_template;
        }

        return $tpl;
    }


    /*
     * @param $templates assoc array ex: array(
     *                                         'subject' => 'blah',
     *                                         'body' => 'blah'
     *                                     )
     * @param $values assoc array es: array(
     *                                          'TICKET_ID' => 24,
     *                                          'TICKET_AUTHOR' => jsmith
     *                                     )
     * @return The same $template array with the replacements done
     */
    public function build_email_template($template, $values)
    {
        foreach($values as $k => $v)
        {
            $regex[]   = "/\b$k\b/";
            $replace[] = "$v"; // ex: "ID" ==> "23" (preserve the surrounding quotes)
        }

        return preg_replace($regex, $replace, $template);
    }


    /*
     * For resetting the templates back to defaults set the vars to ''
     */
    public static function save_email_template($subject_tpl, $body_tpl)
    {
        $conf = new Config;
        $conf->update('email_subject_template', $subject_tpl);
        $conf->update('email_body_template', $body_tpl);
    }


    //@return TRUE if current user have permissions to delete an incident_ticket

    public static function user_tickets_perms($conn, $id, $user = NULL)
    {
        $current_user = (empty($user)) ? Session::get_session_user() : $user;

        //I'm admin
        if (Session::am_i_admin())
        {
            return TRUE;
        }

        $query = ossim_query("SELECT users FROM incident_ticket WHERE id = ".$id);
        $rs    = $conn->Execute($query);

        if (!$rs)
        {
            return $conn->ErrorMsg();
        }
        else
        {
            if (!$rs->EOF)
            {
                $creator = $rs->fields["users"];
            }
        }

        //I'm the submmiter
        if ($creator == $current_user)
        {
            return TRUE;
        }

        //I'm pro_admin and some of my users is the creator
        $conf = $GLOBALS["CONF"];

        if (!$conf)
        {
            $conf = new Ossim_conf();
        }

        $version  = $conf->get_conf("ossim_server_version");
        $pro      = (preg_match("/pro|demo/i",$version)) ? TRUE : FALSE;


        if ($pro && Acl::am_i_proadmin())
        {
            $users_list = Acl::get_my_users($conn, $current_user);

            if (is_array($users_list) && !empty($users_list))
            {
                foreach($users_list as $v)
                {
                    $my_users[] = $v->get_login();
                }

                if (in_array($creator, $my_users))
                {
                    return TRUE;
                }
            }
        }

        return FALSE;
    }


    public function clean_html_tags ($input)
    {
        $input = preg_replace("/\<img (.*?)\>/", "#IMG \\1#", $input);

        $input = str_ireplace("<div", "#DIV#", $input);
        $input = str_ireplace("</div>", "#/DIV#", $input);

        $input = str_ireplace("<span", "#SPAN#", $input);
        $input = str_ireplace("</span>", "#/SPAN#", $input);

        $input = str_ireplace("<ul>", "#UL#", $input);
        $input = str_ireplace("</ul>", "#/UL#", $input);

        $input = str_ireplace("<li>", "#LI#", $input);
        $input = str_ireplace("</li>", "#/LI#", $input);

        $input = str_ireplace("<ol>", "#OL#", $input);
        $input = str_ireplace("</ol>", "#/OL#", $input);

        $input = str_ireplace("<b>", "#B#", $input);
        $input = str_ireplace("</b>", "#/B#", $input);

        $input = str_ireplace("<i>", "#I#", $input);
        $input = str_ireplace("</i>", "#/I#", $input);

        $input = str_ireplace("<u>", "#U#", $input);
        $input = str_ireplace("</u>", "#/U#", $input);

        $input = str_ireplace("<strike>", "#STRIKE#", $input);
        $input = str_ireplace("</strike>", "#/STRIKE#", $input);

        $input = str_ireplace("<p", "#P#", $input);
        $input = str_ireplace("</p>", "#/P#", $input);

        $input = preg_replace("/<h(1|2|3|4|5|6)>/i", "#H$1#", $input);
        $input = preg_replace("/<\/h(1|2|3|4|5|6)>/i", "#/H$1#", $input);

        $input = str_ireplace("<font", "#FONT#", $input);
        $input = str_ireplace("</font>", "#/FONT#", $input);

        $input = str_ireplace("<br>", "#BR#", $input);
        $input = str_ireplace("</br>", "#/BR#", $input);

        $input = str_ireplace("<blockquote>", "#BLOCKQUOTE#", $input);
        $input = str_ireplace("</blockquote>", "#/BLOCKQUOTE#", $input);


        $input = preg_replace ('/<[^>]*>/', '', $input); // clean html tags

        $input = str_ireplace("#DIV#", "<div", $input);
        $input = str_ireplace("#/DIV#", "</div>", $input);

        $input = str_ireplace("#SPAN#", "<span", $input);
        $input = str_ireplace("#/SPAN#", "</span>", $input);

        $input = str_ireplace("#UL#", "<ul>", $input);
        $input = str_ireplace("#/UL#", "</ul>", $input);

        $input = str_ireplace("#LI#", "<li>", $input);
        $input = str_ireplace("#/LI#", "</li>", $input);

        $input = str_ireplace("#OL#", "<ol>", $input);
        $input = str_ireplace("#/OL#", "</ol>", $input);

        $input = str_ireplace("#B#", "<b>", $input);
        $input = str_ireplace("#/B#", "</b>", $input);

        $input = str_ireplace("#I#", "<i>", $input);
        $input = str_ireplace("#/I#", "</i>", $input);

        $input = str_ireplace("#U#", "<u>", $input);
        $input = str_ireplace("#/U#", "</u>", $input);

        $input = str_ireplace("#STRIKE#", "<strike>", $input);
        $input = str_ireplace("#/STRIKE#", "</strike>", $input);

        $input = str_ireplace("#P#", "<p", $input);
        $input = str_ireplace("#/P#", "</p>", $input);

        $input = preg_replace("/#H(1|2|3|4|5|6)#/i", "<h$1>", $input);
        $input = preg_replace("/#\/H(1|2|3|4|5|6)#/i", "</h$1>", $input);

        $input = str_ireplace("#FONT#", "<font", $input);
        $input = str_ireplace("#/FONT#", "</font>", $input);

        $input = str_ireplace("#BR#", "<br>", $input);
        $input = str_ireplace("#/BR#", "</br>", $input);

        $input = str_ireplace("#BLOCKQUOTE#", "<blockquote>", $input);
        $input = str_ireplace("#/BLOCKQUOTE#", "</blockquote>", $input);

        $input = preg_replace("/#IMG (.*?)#/", "<img \\1>", $input);

        return $input;
    }
}

/* End of file incident_ticket.inc */
/* Location: ../include/classes/incident_ticket.inc */
