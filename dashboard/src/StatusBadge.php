<?php

class StatusBadge {
    // Status strings come from several services with differing spellings,
    // so match on a normalised form.
    private const TONES = [
        'complete' => 'success',
        'completed' => 'success',
        'closed' => 'success',
        'inprogress' => 'primary',
        'open' => 'primary',
        'overdue' => 'danger',
        'notstarted' => 'secondary',
    ];

    public static function html($status) {
        $key = strtolower(preg_replace('/[^a-z]/i', '', (string)$status));
        $tone = self::TONES[$key] ?? 'secondary';
        return '<span class="status-pill status-' . $tone . '">' . htmlspecialchars($status) . '</span>';
    }
}
