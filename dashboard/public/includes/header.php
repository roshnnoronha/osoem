<?php
// Relies on ../src/bootstrap.php having already run in the including page
// (sets $authService and $eid, and redirects to index.php if not logged in).
$userInfo = $authService->getUserInfo($eid);
$userName = $userInfo['firstname'] . ' ' . $userInfo['lastname'];
$userInitials = strtoupper(substr($userInfo['firstname'], 0, 1) . substr($userInfo['lastname'], 0, 1));
$currentPage = basename($_SERVER['PHP_SELF']);
?>
<nav class="navbar navbar-expand-md navbar-dark app-navbar">
    <div class="container">
        <a class="navbar-brand d-flex align-items-center" href="dashboard.php">
            <span class="brand-mark">O</span>Osoem
        </a>
        <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbar-collapse"
                aria-controls="navbar-collapse" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
        </button>

        <div class="collapse navbar-collapse" id="navbar-collapse">
            <ul class="navbar-nav me-auto">
                <li class="nav-item">
                    <a class="nav-link <?php echo $currentPage == 'dashboard.php' ? 'active' : ''; ?>" href="dashboard.php">
                        <i class="bi bi-grid me-1"></i> Dashboard
                    </a>
                </li>
                <li class="nav-item">
                    <a class="nav-link <?php echo $currentPage == 'book_time.php' ? 'active' : ''; ?>" href="book_time.php">
                        <i class="bi bi-clock me-1"></i> Book Time
                    </a>
                </li>
            </ul>

            <ul class="navbar-nav">
                <li class="nav-item dropdown">
                    <a href="#" class="nav-link dropdown-toggle d-flex align-items-center gap-2" role="button"
                       data-bs-toggle="dropdown" aria-expanded="false">
                        <span class="avatar"><?php echo htmlspecialchars($userInitials); ?></span>
                        <span><?php echo htmlspecialchars($userName); ?></span>
                    </a>
                    <ul class="dropdown-menu dropdown-menu-end">
                        <li><h6 class="dropdown-header"><?php echo htmlspecialchars($userInfo['email']); ?></h6></li>
                        <li><hr class="dropdown-divider"></li>
                        <li>
                            <a class="dropdown-item" href="logout.php">
                                <i class="bi bi-box-arrow-right"></i> Log out
                            </a>
                        </li>
                    </ul>
                </li>
            </ul>
        </div>
    </div>
</nav>
