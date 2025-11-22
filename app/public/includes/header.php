<?php
if (!isset($authService)) {
    require_once '../src/autoload.php';
    $authService = new AuthService();
}

$eid = $authService->validateSession();
if (!$eid) {
    header('Location: index.php');
    exit();
}

$userInfo = $authService->getUserInfo($eid);
$userName = $userInfo['firstname'] . ' ' . $userInfo['lastname'];
?>

<nav class="navbar navbar-default">
    <div class="container-fluid">
        <div class="navbar-header">
            <button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar-collapse">
                <span class="sr-only">Toggle navigation</span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
            </button>
            <a class="navbar-brand" href="dashboard.php">
                <strong>OsoEM</strong>
            </a>
        </div>

        <div class="collapse navbar-collapse" id="navbar-collapse">
            <ul class="nav navbar-nav">
                <li class="<?php echo basename($_SERVER['PHP_SELF']) == 'dashboard.php' ? 'active' : ''; ?>">
                    <a href="dashboard.php">
                        <span class="glyphicon glyphicon-home"></span> Dashboard
                    </a>
                </li>
            </ul>

            <ul class="nav navbar-nav navbar-right">
                <li class="dropdown">
                    <a href="#" class="dropdown-toggle" data-toggle="dropdown" role="button" aria-haspopup="true" aria-expanded="false">
                        <span class="glyphicon glyphicon-user"></span> 
                        <?php echo htmlspecialchars($userName); ?> 
                        <span class="caret"></span>
                    </a>
                    <ul class="dropdown-menu">
                        <li>
                            <a href="#">
                                <span class="glyphicon glyphicon-cog"></span> Profile Settings
                            </a>
                        </li>
                        <li role="separator" class="divider"></li>
                        <li>
                            <a href="logout.php">
                                <span class="glyphicon glyphicon-log-out"></span> Logout
                            </a>
                        </li>
                    </ul>
                </li>
            </ul>
        </div>
    </div>
</nav>

<style>
/* Custom styles for the navigation */
.navbar-brand {
    font-size: 24px;
    color: #337ab7 !important;
}

.navbar-default .navbar-nav > li > a.disabled {
    color: #ccc !important;
    cursor: not-allowed;
    pointer-events: none;
}

.navbar-default .navbar-nav > .active > a,
.navbar-default .navbar-nav > .active > a:hover,
.navbar-default .navbar-nav > .active > a:focus {
    background-color: #337ab7;
    color: white !important;
}

.dropdown-menu > li > a:hover {
    background-color: #f5f5f5;
}

.glyphicon {
    margin-right: 5px;
}
</style>
